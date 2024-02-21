/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <GeomSerialization/GeomLibsFlatBufferApi.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#include <DgnPlatformInternal/DgnCore/ElementGraphics.fb.h>
#include <DgnPlatformInternal/DgnCore/TextStringPersistence.h>
#include "DgnPlatform/Annotations/TextAnnotationDraw.h"

using namespace flatbuffers;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FaceAttachmentUtil::FromGeometryParams(FaceAttachment& attachment, Render::GeometryParamsCR sourceParams)
    {
    if (!sourceParams.IsLineColorFromSubCategoryAppearance())
        attachment.SetColor(sourceParams.GetLineColor().GetValue(), sourceParams.GetTransparency());

    if (!sourceParams.IsMaterialFromSubCategoryAppearance())
        attachment.SetMaterial(sourceParams.GetMaterialId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FaceAttachmentUtil::ToGeometryParams(FaceAttachment const& attachment, Render::GeometryParamsR faceParams, Render::GeometryParamsCR baseParams)
    {
    faceParams = baseParams;

    if (attachment.GetUseColor())
        {
        faceParams.SetLineColor(ColorDef(attachment.GetColor()));
        faceParams.SetTransparency(attachment.GetTransparency());
        }

    if (attachment.GetUseMaterial())
        faceParams.SetMaterialId(RenderMaterialId(attachment.GetMaterial().GetValueUnchecked()));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricPrimitive::GetLocalCoordinateFrame(TransformR localToWorld) const
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

        case GeometryType::TextString:
            {
            TextStringCR text = *GetAsTextString();

            localToWorld.InitFrom(text.GetOrientation(), text.GetOrigin());
            break;
            }

        case GeometryType::Image:
            {
            auto const& corners = GetAsImage()->GetCorners();
            DPoint3d origin = corners.m_pts[0];
            DVec3d xVec = DVec3d::FromStartEnd(corners.m_pts[0], corners.m_pts[1]);
            DVec3d yVec = DVec3d::FromStartEnd(corners.m_pts[0], corners.m_pts[2]);
            RotMatrix rMatrix = RotMatrix::From2Vectors(xVec, yVec);

            localToWorld.InitFrom(rMatrix, origin);
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricPrimitive::GetLocalRange(DRange3dR localRange, TransformR localToWorld) const
    {
    if (!GetLocalCoordinateFrame(localToWorld))
        return false;

    if (localToWorld.IsIdentity())
        return GetRange(localRange);

    GeometricPrimitivePtr clone;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (GeometryType::BRepEntity == GetGeometryType())
        {
        IBRepEntityPtr instance = GetAsIBRepEntity()->CreateInstance();

        if (!instance.IsValid())
            return false;

        clone = new GeometricPrimitive(instance);
        }
    else
        {
        clone = Clone();
        }

    Transform worldToLocal;

    worldToLocal.InverseOf(localToWorld);
    clone->TransformInPlace(worldToLocal);

    return clone->GetRange(localRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static bool getRange(T const& geom, DRange3dR range, TransformCP transform)
    {
    return nullptr != transform ? geom.GetRange(range, *transform) : geom.GetRange(range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<> bool getRange(ImageGraphicCR geom, DRange3dR range, TransformCP transform)
    {
    range = geom.GetRange();
    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<> bool getRange(PolyfaceQueryCR geom, DRange3dR range, TransformCP transform)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<> bool getRange(MSBsplineSurfaceCR geom, DRange3dR range, TransformCP transform)
    {
    double expansionFactor = 0.2;

    range = geom.TrimmedSurfaceRange (transform, expansionFactor, true);

    // ADO #888596: if mesher fails, try again without boundaries
    if (range.IsNull() && geom.GetNumBounds() > 0)
        {
        auto fullGeom = const_cast<MSBsplineSurfaceR>(geom);
        AutoRestore<int32_t> save(&fullGeom.numBounds, 0);
        range = fullGeom.TrimmedSurfaceRange(transform, expansionFactor, true);
        }

    return !range.IsNull ();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<> bool getRange(IBRepEntityCR geom, DRange3dR range, TransformCP transform)
    {
    range = geom.GetEntityRange();

    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<> bool getRange(TextStringCR text, DRange3dR range, TransformCP transform)
    {
    DRange2dCR textRange = text.GetRange();
    range.low.Init(textRange.low);
    range.high.Init(textRange.high);

    // TextString::GetRange will report the cell box, which typically does /not/ encompass descenders or fancy adornments.
    // The range being computed here directly affects element range, and elements cannot draw outside of their stated range.
    // As such, similar to olden days, artificially pad our reported range to encourage glyph geometry to fit inside of it.
    // This simple padding is a tradeoff between performance and actually computing this specific string's glyph geometry for a tight box.
    double yPad = (text.GetStyle().GetSize().y / 2.0);
    range.low.y -= yPad;
    range.high.y += yPad;

    Transform textTransform = text.ComputeTransform();
    textTransform.Multiply(&range.low, 2);

    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricPrimitive::GetRange(DRange3dR range, TransformCP transform) const
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
            PolyfaceQueryCR geom = *GetAsPolyfaceHeader();

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

        case GeometryType::TextString:
            {
            TextStringCR geom = *GetAsTextString();

            return getRange(geom, range, transform);
            }

        case GeometryType::Image:
            return getRange(*GetAsImage(), range, transform);

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
bool GeometricPrimitive::TransformInPlace(TransformCR transform)
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

        case GeometryType::TextString:
            {
            TextStringR geom = *GetAsTextString();

            geom.ApplyTransform(transform);

            return true;
            }

        case GeometryType::Image:
            GetAsImage()->ApplyTransform(transform);
            return true;

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
bool GeometricPrimitive::IsSameStructureAndGeometry(GeometricPrimitiveCR primitive, double tolerance) const
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

        case GeometryType::BRepEntity:
            {
            IBRepEntityCR geom1 = *GetAsIBRepEntity();
            IBRepEntityCR geom2 = *primitive.GetAsIBRepEntity();

            return geom1.IsSameStructureAndGeometry(geom2, tolerance);
            }

        case GeometryType::TextString: // <- Don't currently need to compare TextString...
        case GeometryType::Image: // <- Don't currently need to compare ImageGraphic...
        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GeometricPrimitive::Clone() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr geom = GetAsICurvePrimitive()->Clone();

            return new GeometricPrimitive(geom);
            }

        case GeometryType::CurveVector:
            {
            CurveVectorPtr geom = GetAsCurveVector()->Clone();

            return new GeometricPrimitive(geom);
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr geom = GetAsISolidPrimitive()->Clone();

            return new GeometricPrimitive(geom);
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderPtr geom = GetAsPolyfaceHeader()->Clone();

            return new GeometricPrimitive(geom);
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr geom = GetAsMSBsplineSurface()->Clone();

            return new GeometricPrimitive(geom);
            }

        case GeometryType::BRepEntity:
            {
            IBRepEntityPtr geom = GetAsIBRepEntity()->Clone();

            return new GeometricPrimitive(geom);
            }

        case GeometryType::TextString:
            {
            TextStringPtr geom = GetAsTextString()->Clone();

            return new GeometricPrimitive(geom);
            }

        case GeometryType::Image:
            return new GeometricPrimitive(GetAsImage()->Clone());

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
void GeometricPrimitive::AddToGraphic(Render::GraphicBuilderR graphic) const
    {
    switch (GetGeometryType())
        {
        case GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr geom = GetAsICurvePrimitive();
            CurveVectorPtr curveGeom = CurveVector::Create(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == geom->GetCurvePrimitiveType() ? CurveVector::BOUNDARY_TYPE_None : CurveVector::BOUNDARY_TYPE_Open, geom);

            graphic.AddCurveVector(*curveGeom, false);
            break;
            }

        case GeometryType::CurveVector:
            {
            CurveVectorCR geom = *GetAsCurveVector();

            graphic.AddCurveVector(geom, false);
            break;
            }

        case GeometryType::SolidPrimitive:
            {
            ISolidPrimitiveCR geom = *GetAsISolidPrimitive();

            graphic.AddSolidPrimitive(geom);
            break;
            }

        case GeometryType::Polyface:
            {
            PolyfaceHeaderCR geom = *GetAsPolyfaceHeader();

            graphic.AddPolyface(geom, false);
            break;
            }

        case GeometryType::BsplineSurface:
            {
            MSBsplineSurfaceCR geom = *GetAsMSBsplineSurface();

            graphic.AddBSplineSurface(geom);
            break;
            }

        case GeometryType::BRepEntity:
            {
            IBRepEntityCR geom = *GetAsIBRepEntity();

            graphic.AddBody(geom);
            break;
            }

        case GeometryType::TextString:
            {
            TextStringCR geom = *GetAsTextString();

            graphic.AddTextString(geom);
            break;
            }

        case GeometryType::Image:
            graphic.AddImage(*GetAsImage());
            break;

        default:
            {
            BeAssert(false);
            break;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricPrimitive::IsSolid() const
    {
    switch (GetGeometryType())
        {
        case GeometryType::SolidPrimitive:
            return GetAsISolidPrimitive()->GetCapped();

        case GeometryType::Polyface:
            {
            auto polyface = GetAsPolyfaceHeader();
            switch (polyface->GetExpectedClosure())
                {
                case 1:
                    return false;
                case 2:
                    return true;
                }
            return polyface->IsClosedByEdgePairing();
            }

        case GeometryType::BRepEntity:
            return IBRepEntity::EntityType::Solid == GetAsIBRepEntity()->GetEntityType();

        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricPrimitive::IsSheet() const
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
            return !IsSolid();

        case GeometryType::BRepEntity:
            return IBRepEntity::EntityType::Sheet == GetAsIBRepEntity()->GetEntityType();

        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricPrimitive::IsWire() const
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
IBRepEntity::EntityType GeometricPrimitive::GetBRepEntityType() const
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
            return IsSolid() ? IBRepEntity::EntityType::Solid : IBRepEntity::EntityType::Sheet;

        case GeometryType::BRepEntity:
            return GetAsIBRepEntity()->GetEntityType();

        default:
            return IBRepEntity::EntityType::Invalid;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitive::GeometricPrimitive(ICurvePrimitivePtr const& source) {m_type = GeometryType::CurvePrimitive; m_data = source;}
GeometricPrimitive::GeometricPrimitive(CurveVectorPtr const& source) {m_type = GeometryType::CurveVector; m_data = source;}
GeometricPrimitive::GeometricPrimitive(ISolidPrimitivePtr const& source) {m_type = GeometryType::SolidPrimitive; m_data = source;}
GeometricPrimitive::GeometricPrimitive(MSBsplineSurfacePtr const& source) {m_type = GeometryType::BsplineSurface; m_data = source;}
GeometricPrimitive::GeometricPrimitive(PolyfaceHeaderPtr const& source) {m_type = GeometryType::Polyface; m_data = source;}
GeometricPrimitive::GeometricPrimitive(IBRepEntityPtr const& source) {m_type = GeometryType::BRepEntity; m_data = source;}
GeometricPrimitive::GeometricPrimitive(TextStringPtr const& source) {m_type = GeometryType::TextString; m_data = source;}
GeometricPrimitive::GeometricPrimitive(ImageGraphicPtr const& source) {m_type = GeometryType::Image; m_data = source;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GeometricPrimitive::Create(ICurvePrimitivePtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(CurveVectorPtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(ISolidPrimitivePtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(MSBsplineSurfacePtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(PolyfaceHeaderPtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(IBRepEntityPtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(TextStringPtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(ImageGraphicPtr const& source) {return source.IsValid() ? new GeometricPrimitive(source) : nullptr;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GeometricPrimitive::Create(ICurvePrimitiveCR source) {ICurvePrimitivePtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(CurveVectorCR source) {CurveVectorPtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(ISolidPrimitiveCR source) {ISolidPrimitivePtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(MSBsplineSurfaceCR source) {MSBsplineSurfacePtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(PolyfaceQueryCR source) {PolyfaceHeaderPtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(IBRepEntityCR source) {IBRepEntityPtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(TextStringCR source) {TextStringPtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(ImageGraphicCR source) {return Create(source.Clone());}

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GeometricPrimitive::Create(DEllipse3dCR source) {ICurvePrimitivePtr curve = ICurvePrimitive::CreateArc(source); return curve.IsValid() ? Create(curve) : nullptr;}
GeometricPrimitivePtr GeometricPrimitive::Create(DgnBoxDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnBox(source); return solid.IsValid() ? Create(solid) : nullptr;}
GeometricPrimitivePtr GeometricPrimitive::Create(DgnConeDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnCone(source); return solid.IsValid() ? Create(solid) : nullptr;}
GeometricPrimitivePtr GeometricPrimitive::Create(DgnSphereDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnSphere(source); return solid.IsValid() ? Create(solid) : nullptr;}
GeometricPrimitivePtr GeometricPrimitive::Create(DgnTorusPipeDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnTorusPipe(source); return solid.IsValid() ? Create(solid) : nullptr;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitive::GeometryType GeometricPrimitive::GetGeometryType() const {return m_type;}
ICurvePrimitivePtr GeometricPrimitive::GetAsICurvePrimitive() const {return (GeometryType::CurvePrimitive == m_type ? static_cast <ICurvePrimitiveP> (m_data.get()) : nullptr);}
CurveVectorPtr GeometricPrimitive::GetAsCurveVector() const {return (GeometryType::CurveVector == m_type ? static_cast <CurveVectorP> (m_data.get()) : nullptr);}
ISolidPrimitivePtr GeometricPrimitive::GetAsISolidPrimitive() const {return (GeometryType::SolidPrimitive == m_type ? static_cast <ISolidPrimitiveP> (m_data.get()) : nullptr);}
MSBsplineSurfacePtr GeometricPrimitive::GetAsMSBsplineSurface() const {return (GeometryType::BsplineSurface == m_type ? static_cast <RefCountedMSBsplineSurface*> (m_data.get()) : nullptr);}
PolyfaceHeaderPtr GeometricPrimitive::GetAsPolyfaceHeader() const {return (GeometryType::Polyface == m_type ? static_cast <PolyfaceHeaderP> (m_data.get()) : nullptr);}
IBRepEntityPtr GeometricPrimitive::GetAsIBRepEntity() const {return (GeometryType::BRepEntity == m_type ? static_cast <IBRepEntityP> (m_data.get()) : nullptr);}
TextStringPtr GeometricPrimitive::GetAsTextString() const {return (GeometryType::TextString == m_type ? static_cast <TextStringP> (m_data.get()) : nullptr);}
ImageGraphicPtr GeometricPrimitive::GetAsImage() const {return GeometryType::Image == m_type ? static_cast<ImageGraphicP>(m_data.get()) : nullptr;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Operation::IsGeometryOp() const
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
        case OpCode::TextString:
        case OpCode::Image:
            return true;

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(Operation const& egOp)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(DPoint2dCP pts, size_t nPts, int8_t boundary)
    {
    FlatBufferBuilder fbb;

    auto coords = fbb.CreateVectorOfStructs((FB::DPoint2d const*) pts, nPts);

    FB::PointPrimitive2dBuilder builder(fbb);

    builder.add_coords(coords);
    builder.add_boundary((FB::BoundaryType) boundary);

    auto mloc = builder.Finish();

    fbb.Finish(mloc);
    Append(Operation(OpCode::PointPrimitive2d, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(DPoint3dCP pts, size_t nPts, int8_t boundary)
    {
    FlatBufferBuilder fbb;

    auto coords = fbb.CreateVectorOfStructs((FB::DPoint3d const*) pts, nPts);

    FB::PointPrimitiveBuilder builder(fbb);

    builder.add_coords(coords);
    builder.add_boundary((FB::BoundaryType) boundary);

    auto mloc = builder.Finish();

    fbb.Finish(mloc);
    Append(Operation(OpCode::PointPrimitive, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(DRange3dCR range)
    {
    FlatBufferBuilder fbb;

    auto coords = fbb.CreateVectorOfStructs((FB::DPoint3d const*) &range.low, 2);

    FB::PointPrimitiveBuilder builder(fbb);

    builder.add_coords(coords);

    auto mloc = builder.Finish();

    fbb.Finish(mloc);
    Append(Operation(OpCode::SubGraphicRange, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(DEllipse3dCR arc, int8_t boundary)
    {
    FlatBufferBuilder fbb;

    auto mloc = FB::CreateArcPrimitive(fbb, (FB::DPoint3d const*) &arc.center, (FB::DVec3d const*) &arc.vector0, (FB::DVec3d const*) &arc.vector90, arc.start, arc.sweep, (FB::BoundaryType) boundary);

    fbb.Finish(mloc);
    Append(Operation(OpCode::ArcPrimitive, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool hasDisconnectPoint(DPoint3dCP pts, size_t nPts)
    {
    for (size_t iPt = 0; iPt < nPts; ++iPt)
        {
        if (pts[iPt].IsDisconnect())
            {
            BeAssert(false); // STOP USING THIS! Create a BOUNDARY_TYPE_None CurveVector with disjoint pieces...
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool hasDisconnectPoint(ICurvePrimitiveCR curve)
    {
    switch (curve.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            return hasDisconnectPoint(curve.GetLineCP()->point, 2);

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return hasDisconnectPoint(&curve.GetLineStringCP()->front(), curve.GetLineStringCP()->size());

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            return hasDisconnectPoint(&curve.GetPointStringCP()->front(), curve.GetPointStringCP()->size());

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isInvalidCurveVector(CurveVectorCR curves)
    {
    if (curves.IsUnionRegion())
        {
        for (ICurvePrimitivePtr curve : curves)
            {
            if (curve.IsNull())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                return true; // Each loop must be a child curve bvector (a closed loop or parity region)...

            CurveVectorCP childCurves = curve->GetChildCurveVectorCP();

            if ((!childCurves->IsClosedPath() && !childCurves->IsParityRegion()) || isInvalidCurveVector(*childCurves))
                return true;
            }
        }
    else if (curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve : curves)
            {
            if (curve.IsNull())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                return true; // Each loop must be a child curve bvector (a closed loop)...

            CurveVectorCP childCurves = curve->GetChildCurveVectorCP();

            if (!childCurves->IsClosedPath() || isInvalidCurveVector(*childCurves))
                return true;
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve : curves)
            {
            if (!curve.IsValid())
                continue;

            switch (curve->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                    {
                    if (isInvalidCurveVector(*curve->GetChildCurveVectorCP()))
                        return true;
                    break;
                    }

                default:
                    {
                    if (hasDisconnectPoint(*curve))
                        return true;
                    break;
                    }
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Writer::AppendSimplified(ICurvePrimitiveCR curvePrimitive, bool isClosed, bool is3d)
    {
    // Special case single/simple curve primitives to avoid having to call new during draw...
    switch (curvePrimitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCP segment = curvePrimitive.GetLineCP();

            if (hasDisconnectPoint(segment->point, 2))
                return false;

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

            if (hasDisconnectPoint(&points->front(), points->size()))
                return false;

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

            if (hasDisconnectPoint(&points->front(), points->size()))
                return false;

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
            {
            BeAssert(!isClosed);
            Append(curvePrimitive);

            return true;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Writer::AppendSimplified(CurveVectorCR curves, bool is3d)
    {
    // NOTE: Special case to avoid having to call new during draw...
    switch (curves.HasSingleCurvePrimitive())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            return AppendSimplified(*curves.front(), false, is3d); // Never closed...

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            return AppendSimplified(*curves.front(), curves.IsClosedPath(), is3d);

        default:
            if (isInvalidCurveVector(curves)) return false; Append(curves); return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Writer::AppendSimplified(GeometricPrimitiveCR geom, bool is3d)
    {
    switch (geom.GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurvePrimitive:
            return AppendSimplified(*geom.GetAsICurvePrimitive(), false, is3d);

        case GeometricPrimitive::GeometryType::CurveVector:
            return AppendSimplified(*geom.GetAsCurveVector(), is3d);

        default:
            Append(geom); return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(CurveVectorCR curves)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(ICurvePrimitiveCR curvePrimitive)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes(curvePrimitive, buffer);

    if (0 == buffer.size())
        {
        BeAssert(false);
        return;
        }

    Append(Operation(OpCode::CurvePrimitive, (uint32_t) buffer.size(), &buffer.front()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(PolyfaceQueryCR meshData)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(ISolidPrimitiveCR solid)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(MSBsplineSurfaceCR surface)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::AppendNonBRepRepresentation(IBRepEntityCR entity, IFacetOptionsCP facetOptIn)
    {
    // NOTE: Append non-brep representation for when we don't want Parasolid...
    switch (entity.GetEntityType())
        {
        case IBRepEntity::EntityType::Wire:
            {
            // Save wire body as CurveVector...
            CurveVectorPtr curve = T_HOST.GetBRepGeometryAdmin()._BodyToCurveVector(entity);

            if (curve.IsValid())
                Append(*curve);
            else
                BeAssert(false && "no curve created for wire body");
            break;
            }

        case IBRepEntity::EntityType::Sheet:
            {
            // Save sheet body that is a single planar face as CurveVector...
            CurveVectorPtr curve = T_HOST.GetBRepGeometryAdmin()._BodyToCurveVector(entity);

            if (curve.IsValid())
                {
                Append(*curve);
                break;
                }

            // Fall through...
            }

        case IBRepEntity::EntityType::Solid:
            {
            IFacetOptionsPtr facetOpt;

            if (nullptr == facetOptIn)
                {
                facetOpt = IFacetOptions::CreateForCurves();

                facetOpt->SetAngleTolerance(0.2); // NOTE: This is the value XGraphics "optimize" used...
                facetOpt->SetNormalsRequired(true);
                facetOpt->SetParamsRequired(true);
                facetOpt->SetBRepConcurrentFacetting(false); // Probably no one wants this by default...
                }
            else
                {
                facetOpt = facetOptIn->Clone();
                }

            IFaceMaterialAttachmentsCP attachments = entity.GetFaceMaterialAttachments();

            if (nullptr != attachments)
                {
                bvector<PolyfaceHeaderPtr> polyfaces;
                bvector<FaceAttachment> params;

                T_HOST.GetBRepGeometryAdmin()._FacetEntity(entity, polyfaces, params, *facetOpt);

                if (polyfaces.empty())
                    {
                    BeAssert(false && "no facets produced for solid or sheet body");
                    break;
                    }

                for (size_t i = 0; i < polyfaces.size(); i++)
                    {
                    if (0 == polyfaces[i]->GetPointCount())
                        continue;

                    GeometryParams  faceParams, baseParamsIgnored;

                    FaceAttachmentUtil::ToGeometryParams(params[i], faceParams, baseParamsIgnored);
                    Append(faceParams, true, true); // We don't support allowing sub-category to vary by FaceAttachment...and we didn't initialize it...

                    polyfaces[i]->NormalizeParameters(); // Normalize uv parameters or materials won't have correct scale...
                    Append(*polyfaces[i]);
                    }
                }
            else
                {
                PolyfaceHeaderPtr polyface = T_HOST.GetBRepGeometryAdmin()._FacetEntity(entity, *facetOpt);

                if (!polyface.IsValid())
                    {
                    BeAssert(false && "no facets produced for solid or sheet body");
                    break;
                    }

                polyface->NormalizeParameters(); // Normalize uv parameters or materials won't have correct scale...
                Append(*polyface);
                }

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Writer::AppendNonBRepRepresentation(Operation const& egOp, IFacetOptionsCP facetOptIn)
    {
    GeometryStreamIO::Reader reader(m_db);
    IBRepEntityPtr entityPtr;

    if (!reader.Get(egOp, entityPtr))
        return false;

    AppendNonBRepRepresentation(*entityPtr, facetOptIn);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(IBRepEntityCR entity)
    {
    size_t      bufferSize = 0;
    uint8_t*    buffer = nullptr;

    // NOTE: Only sheets and solids should be persisted as breps, display code only facets...
    if (IBRepEntity::EntityType::Wire == entity.GetEntityType() || SUCCESS != T_HOST.GetBRepGeometryAdmin()._SaveEntityToMemory(&buffer, bufferSize, entity))
        {
        AppendNonBRepRepresentation(entity);
        return;
        }

    IFaceMaterialAttachmentsCP attachments = entity.GetFaceMaterialAttachments();
    bvector<FB::FaceSymbology> fbSymbVec;

    if (nullptr != attachments)
        {
        T_FaceAttachmentsVec const& faceAttachmentsVec = attachments->_GetFaceAttachmentsVec();

        for (FaceAttachment attachment : faceAttachmentsVec)
            {
            FB::DPoint2d uv(0.0, 0.0);
            bool useColor = attachment.GetUseColor();
            bool useMaterial = attachment.GetUseMaterial();

            FB::FaceSymbology  fbSymb(useColor, useMaterial,
                                      useColor ? attachment.GetColor() : 0,
                                      useMaterial ? attachment.GetMaterial().GetValueUnchecked() : 0,
                                      useColor ? attachment.GetTransparency() : 0, uv);

            fbSymbVec.push_back(fbSymb);
            }
        }

    FlatBufferBuilder fbb;

    auto entityData = fbb.CreateVector(buffer, bufferSize);
    auto faceSymb = 0 != fbSymbVec.size() ? fbb.CreateVectorOfStructs(&fbSymbVec.front(), fbSymbVec.size()) : 0;

    FB::BRepDataBuilder builder(fbb);
    Transform entityTransform = entity.GetEntityTransform();

    builder.add_entityTransform((FB::Transform*) &entityTransform);
    builder.add_brepType((FB::BRepType) entity.GetEntityType()); // Allow possibility of checking type w/o expensive restore of brep...
    builder.add_entityData(entityData);

    if (nullptr != attachments)
        builder.add_symbology(faceSymb);

    auto mloc = builder.Finish();

    fbb.Finish(mloc);
    Append(Operation(OpCode::ParasolidBRep, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(DgnGeometryPartId geomPart, TransformCP geomToElem)
    {
    if (nullptr == geomToElem || geomToElem->IsIdentity())
        {
        FlatBufferBuilder fbb;

        auto mloc = FB::CreateGeometryPart(fbb, geomPart.GetValueUnchecked());

        fbb.Finish(mloc);
        Append(Operation(OpCode::GeometryPartInstance, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        return;
        }

    double              scale;
    DPoint3d            origin;
    RotMatrix           rMatrix, deScaledMatrix;
    YawPitchRollAngles  angles;

    geomToElem->GetTranslation(origin);
    geomToElem->GetMatrix(rMatrix);

    if (!rMatrix.IsRigidSignedScale(deScaledMatrix, scale))
        scale = 1.0;

    BeAssert(scale > 0.0); // Mirror not allowed...

    YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix);

    FlatBufferBuilder fbb;

    auto mloc = FB::CreateGeometryPart(fbb, geomPart.GetValueUnchecked(), (FB::DPoint3d*) &origin, angles.GetYaw().Degrees(), angles.GetPitch().Degrees(), angles.GetRoll().Degrees(), fabs(scale));

    fbb.Finish(mloc);
    Append(Operation(OpCode::GeometryPartInstance, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(GeometryParamsCR elParams, bool ignoreSubCategory, bool is3d)
    {
    bool    useColor  = !elParams.IsLineColorFromSubCategoryAppearance();
    bool    useWeight = !elParams.IsWeightFromSubCategoryAppearance();
    bool    useStyle  = !elParams.IsLineStyleFromSubCategoryAppearance();
    int32_t priority  = is3d ? 0 : elParams.GetDisplayPriority(); // Make sure we don't store a value for 3d geometry...

    if (useColor || useWeight || useStyle || 0.0 != elParams.GetTransparency() || 0 != priority || DgnGeometryClass::Primary != elParams.GetGeometryClass())
        {
        FlatBufferBuilder fbb;

        auto mloc = FB::CreateBasicSymbology(fbb, ignoreSubCategory ? 0 : elParams.GetSubCategoryId().GetValueUnchecked(),
                                             useColor ? elParams.GetLineColor().GetValue() : 0,
                                             useWeight ? elParams.GetWeight() : 0,
                                             useStyle && nullptr != elParams.GetLineStyle() ? elParams.GetLineStyle()->GetStyleId().GetValueUnchecked() : 0,
                                             elParams.GetTransparency(), priority, (FB::GeometryClass) elParams.GetGeometryClass(),
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

    if (useStyle && nullptr != elParams.GetLineStyle() && nullptr != elParams.GetLineStyle()->GetStyleParams())
        {
        FlatBufferBuilder   fbb;
        LineStyleParamsCP   lsParams = elParams.GetLineStyle()->GetStyleParams();
        YawPitchRollAngles  angles;

        YawPitchRollAngles::TryFromRotMatrix(angles, lsParams->rMatrix);
        auto modifiers = FB::CreateLineStyleModifiers(fbb, lsParams->modifiers, lsParams->scale, lsParams->dashScale, lsParams->gapScale, lsParams->startWidth, lsParams->endWidth, lsParams->distPhase, lsParams->fractPhase,
                                                      (FB::DPoint3d const*)&lsParams->normal, angles.GetYaw().Degrees(), angles.GetPitch().Degrees(), angles.GetRoll().Degrees());
        fbb.Finish(modifiers);
        Append(Operation(OpCode::LineStyleModifiers, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }

    if (FillDisplay::Never != elParams.GetFillDisplay())
        {
        FlatBufferBuilder fbb;

        if (nullptr != elParams.GetGradient())
            {
            GradientSymbCR    gradient = *elParams.GetGradient();
            bvector<uint32_t> keyColors;
            bvector<double>   keyValues;

            for (uint32_t i=0; i < gradient.GetNKeys(); ++i)
                {
                double   keyValue;
                ColorDef keyColor;

                gradient.GetKey(keyColor, keyValue, i);

                keyColors.push_back(keyColor.GetValue());
                keyValues.push_back(keyValue);
                }

            auto colors = fbb.CreateVector(keyColors);
            auto values = fbb.CreateVector(keyValues);
            flatbuffers::Offset<FB::ThematicSettings>   thematicSettingsOffset = 0;
            if (gradient.GetMode() == GradientSymb::Mode::Thematic && gradient.GetThematicSettings().IsValid())
                {
                auto& thematicSettings = *gradient.GetThematicSettings();

                thematicSettingsOffset = FB::CreateThematicSettings(fbb,
                                                                   thematicSettings.GetStepCount(),
                                                                   thematicSettings.GetMargin(),
                                                                   thematicSettings.GetMarginColor().GetValue(),
                                                                   (uint32_t) thematicSettings.GetMode(),
                                                                   (uint32_t) thematicSettings.GetColorScheme(),
                                                                   (FB::DRange1d const*) &thematicSettings.GetRange());
                }

            auto mloc = FB::CreateAreaFill(fbb, (FB::FillDisplay) elParams.GetFillDisplay(),
                                           0, 0, 0, elParams.GetFillTransparency(),
                                           (FB::GradientMode) gradient.GetMode(), gradient.GetFlags(),
                                           gradient.GetAngle(), gradient.GetTint(), gradient.GetShift(),
                                           colors, values, thematicSettingsOffset);
            fbb.Finish(mloc);
            }
        else
            {
            bool outline = false;
            bool isBgFill = elParams.IsFillColorFromViewBackground(&outline);
            bool useFillColor = !isBgFill && !elParams.IsFillColorFromSubCategoryAppearance();

            auto mloc = FB::CreateAreaFill(fbb, (FB::FillDisplay) elParams.GetFillDisplay(),
                                           useFillColor ? elParams.GetFillColor().GetValue() : 0, useFillColor,
                                           isBgFill ? (outline ? 2 : 1) : 0, elParams.GetFillTransparency());
            fbb.Finish(mloc);
            }

        Append(Operation(OpCode::AreaFill, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }

    PatternParamsCP pattern;

    if (nullptr != (pattern = elParams.GetPatternParams()))
        {
        FlatBufferBuilder fbb;

        bvector<flatbuffers::Offset<FB::DwgHatchDefLine>> defLineOffsets;

        if (0 != pattern->GetDwgHatchDef().size())
            {
            for (auto defLine : pattern->GetDwgHatchDef())
                {
                FB::DwgHatchDefLineBuilder dashBuilder(fbb);

                auto dashes = fbb.CreateVector(defLine.m_dashes, defLine.m_nDashes);

                dashBuilder.add_angle(defLine.m_angle);
                dashBuilder.add_through((FB::DPoint2d*) &defLine.m_through);
                dashBuilder.add_offset((FB::DPoint2d*) &defLine.m_offset);
                dashBuilder.add_dashes(dashes);

                defLineOffsets.push_back(dashBuilder.Finish());
                }
            }

        flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<FB::DwgHatchDefLine>>> fbDefLines;

        if (0 != defLineOffsets.size())
            fbDefLines = fbb.CreateVector(defLineOffsets);

        FB::AreaPatternBuilder builder(fbb);

        builder.add_origin((FB::DPoint3d const*) &pattern->GetOrigin());

        if (!pattern->GetOrientation().IsIdentity())
            builder.add_rotation((FB::RotMatrix const*) &pattern->GetOrientation());

        if (pattern->GetSymbolId().IsValid())
            {
            builder.add_space1(pattern->GetPrimarySpacing());
            builder.add_space2(pattern->GetSecondarySpacing());
            builder.add_angle1(pattern->GetPrimaryAngle());
            builder.add_scale(pattern->GetScale());
            builder.add_symbolId(pattern->GetSymbolId().GetValueUnchecked());
            }
        else if (0 != pattern->GetDwgHatchDef().size())
            {
            builder.add_angle1(pattern->GetPrimaryAngle()); // NOTE: angle/scale baked into hatch def lines, saved for placement info...
            builder.add_scale(pattern->GetScale());
            builder.add_defLine(fbDefLines);
            }
        else
            {
            builder.add_space1(pattern->GetPrimarySpacing());
            builder.add_space2(pattern->GetSecondarySpacing());
            builder.add_angle1(pattern->GetPrimaryAngle());
            builder.add_angle2(pattern->GetSecondaryAngle());
            }

        if (pattern->GetUseColor())
            {
            builder.add_useColor(true);
            builder.add_color(pattern->GetColor().GetValue());
            }

        if (pattern->GetUseWeight())
            {
            builder.add_useWeight(true);
            builder.add_weight(pattern->GetWeight());
            }

        if (pattern->GetInvisibleBoundary())
            builder.add_invisibleBoundary(true);

        if (pattern->GetSnappable())
            builder.add_snappable(true);

        auto mloc = builder.Finish();

        fbb.Finish(mloc);
        Append(Operation(OpCode::Pattern, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }

    // NEEDSWORK_WIP_MATERIAL - Not sure what we need to store per-geometry...
    //                          I assume we'll still need optional uv settings even when using sub-category material.
    //                          So we need a way to check for that case as we can't call GetMaterial
    //                          when !useMaterial because GeometryParams::Resolve hasn't been called...
    bool useMaterial = is3d && !elParams.IsMaterialFromSubCategoryAppearance();
    auto trans2x3 = elParams.GetMaterialUVDetail().GetTransform();
    if (useMaterial || nullptr != trans2x3)
        {
        FlatBufferBuilder fbb;

        auto mloc = FB::CreateMaterial(fbb, useMaterial, useMaterial && elParams.GetMaterialId().IsValid() ? elParams.GetMaterialId().GetValueUnchecked() : 0, nullptr, nullptr, 0.0, 0.0, 0.0, reinterpret_cast<FB::Trans2x3 const*>(trans2x3));
        fbb.Finish(mloc);

        Append(Operation(OpCode::Material, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(GeometricPrimitiveCR elemGeom)
    {
    switch (elemGeom.GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurvePrimitive:
            Append(*elemGeom.GetAsICurvePrimitive());
            break;

        case GeometricPrimitive::GeometryType::CurveVector:
            Append(*elemGeom.GetAsCurveVector());
            break;

        case GeometricPrimitive::GeometryType::SolidPrimitive:
            Append(*elemGeom.GetAsISolidPrimitive());
            break;

        case GeometricPrimitive::GeometryType::Polyface:
            Append(*elemGeom.GetAsPolyfaceHeader());
            break;

        case GeometricPrimitive::GeometryType::BsplineSurface:
            Append(*elemGeom.GetAsMSBsplineSurface());
            break;

        case GeometricPrimitive::GeometryType::BRepEntity:
            Append(*elemGeom.GetAsIBRepEntity());
            break;

        case GeometricPrimitive::GeometryType::TextString:
            Append(*elemGeom.GetAsTextString());
            break;

        case GeometricPrimitive::GeometryType::Image:
            Append(*elemGeom.GetAsImage());
            break;

        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(TextStringCR text)
    {
    bvector<Byte> data;
    if (SUCCESS != TextStringPersistence::EncodeAsFlatBuf(data, text, m_db, TextStringPersistence::FlatBufEncodeOptions::IncludeGlyphLayoutData))
        return;

    Append(Operation(OpCode::TextString, (uint32_t)data.size(), &data[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(ImageGraphicCR img)
    {
    if (!img.GetTextureId().IsValid())
        return;

    FlatBufferBuilder fbb;
    FB::ImageBuilder builder(fbb);

    auto const* pts = img.GetCorners().m_pts;
    builder.add_tileCorner0((FB::DPoint3d const*) pts + 0);
    builder.add_tileCorner1((FB::DPoint3d const*) pts + 1);
    builder.add_tileCorner2((FB::DPoint3d const*) pts + 2);
    builder.add_tileCorner3((FB::DPoint3d const*) pts + 3);
    builder.add_drawBorder(img.HasBorder());
    builder.add_textureId(static_cast<int64_t>(img.GetTextureId().GetValueUnchecked()));

    auto mloc = builder.Finish();
    fbb.Finish(mloc);
    Append(Operation(OpCode::Image, static_cast<uint32_t>(fbb.GetSize()), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, DPoint2dCP& pts, int& nPts, int8_t& boundary) const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, DPoint3dCP& pts, int& nPts, int8_t& boundary) const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, DRange3dR range) const
    {
    if (OpCode::SubGraphicRange != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::PointPrimitive>(egOp.m_data);

    if (ppfb->coords()->Length() < 2) // NOTE: 6 points were erroneously stored originally, so use < not = check...
        return false;

    memcpy(&range, (DPoint3dCP) ppfb->coords()->Data(), sizeof(range));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, DEllipse3dR arc, int8_t& boundary) const
    {
    if (OpCode::ArcPrimitive != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::ArcPrimitive>(egOp.m_data);

    arc.InitFromVectors(*((DPoint3dCP) ppfb->center()), *((DVec3dCP) ppfb->vector0()), *((DVec3dCP) ppfb->vector90()), ppfb->start(), ppfb->sweep());
    boundary = (int8_t) ppfb->boundary();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, ICurvePrimitivePtr& curve, bool applyValidation) const
    {
    if (OpCode::CurvePrimitive != egOp.m_opCode)
        return false;

    curve = BentleyGeometryFlatBuffer::BytesToCurvePrimitive(egOp.m_data, applyValidation);

    return curve.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, CurveVectorPtr& curves, bool applyValidation) const
    {
    if (OpCode::CurveVector != egOp.m_opCode)
        return false;

    curves = BentleyGeometryFlatBuffer::BytesToCurveVector(egOp.m_data, applyValidation);

    return curves.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool fieldHasIndexErrors(const int32_t* indices, int32_t indexCount, int32_t valueCount)
    {
    for (int32_t i = 0; i < indexCount; ++i)
        {
        int absIndex = abs(indices[i]);
        if (absIndex > valueCount) // 1-based so absIndex == valueCount is OK
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// There are some unknown number of iModels with invalid Polyfaces. We have better checking on
// serialization now, but need to prevent crashing downstream when loading these problematic files.
// PolyfaceQuery::HasIndexErrors() has minor overhead inappropriate for use on each load; this is
// a simplified version that profiling shows does not affect loading time.
enum class MeshAuditResult
    {
    NoIssues = 0,
    IssuesWereFixed = 1,
    CouldNotFix = 2
    };
static MeshAuditResult auditMesh(PolyfaceQueryCarrier& mesh)
    {
    if (mesh.GetMeshStyle() != MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
        return MeshAuditResult::NoIssues; // nothing to validate in other styles

    int32_t indexCount = (int32_t)mesh.GetPointIndexCount();
    if (indexCount == 0)
        return MeshAuditResult::NoIssues; // not great, but won't break anything

    const int32_t* pointIndices = mesh.GetPointIndexCP();
    if (fieldHasIndexErrors(pointIndices, indexCount, (int32_t)mesh.GetPointCount()))
        return MeshAuditResult::CouldNotFix; // garbage data, can't fix outside of dropping triangles

    MeshAuditResult auditResult = MeshAuditResult::NoIssues;

    const int32_t* normalIndices = mesh.GetNormalIndexCP();
    if (normalIndices && fieldHasIndexErrors(normalIndices, indexCount, (int32_t)mesh.GetNormalCount()))
        {
        mesh.RemoveNormals();
        auditResult = MeshAuditResult::IssuesWereFixed;
        }

    const int32_t* paramIndices = mesh.GetParamIndexCP();
    if (paramIndices && fieldHasIndexErrors(paramIndices, indexCount, (int32_t)mesh.GetParamCount()))
        {
        mesh.RemoveParams();
        auditResult = MeshAuditResult::IssuesWereFixed;
        }

    const int32_t* colorIndices = mesh.GetColorIndexCP();
    if (colorIndices && fieldHasIndexErrors(colorIndices, indexCount, (int32_t)mesh.GetColorCount()))
        {
        mesh.RemoveColors();
        auditResult = MeshAuditResult::IssuesWereFixed;
        }

    return auditResult;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, PolyfaceQueryCarrier& meshData, bool applyValidation) const
    {
    if (OpCode::Polyface != egOp.m_opCode)
        return false;

    if (!BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier(egOp.m_data, meshData, applyValidation))
        return false;

    MeshAuditResult auditResult = auditMesh(meshData);
    if (MeshAuditResult::CouldNotFix == auditResult)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, ISolidPrimitivePtr& solid, bool applyValidation) const
    {
    if (OpCode::SolidPrimitive != egOp.m_opCode)
        return false;

    solid = BentleyGeometryFlatBuffer::BytesToSolidPrimitive(egOp.m_data, applyValidation);

    return solid.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, MSBsplineSurfacePtr& surface, bool applyValidation) const
    {
    if (OpCode::BsplineSurface != egOp.m_opCode)
        return false;

    surface = BentleyGeometryFlatBuffer::BytesToMSBsplineSurface(egOp.m_data, applyValidation);

    return surface.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, IBRepEntityPtr& entity) const
    {
    if (OpCode::ParasolidBRep != egOp.m_opCode)
        return false;

    entity = ReadBRepEntity(egOp.m_data);
    return entity.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr GeometryStreamIO::Reader::ReadBRepEntity(uint8_t const* data)
    {
    IBRepEntityPtr entity;

    auto ppfb = flatbuffers::GetRoot<FB::BRepData>(data);

    // NOTE: It's possible to check ppfb->brepType() to avoid calling restore in order to check type...
    if (SUCCESS != T_HOST.GetBRepGeometryAdmin()._RestoreEntityFromMemory(entity, ppfb->entityData()->Data(), ppfb->entityData()->Length(), *((TransformCP) ppfb->entityTransform())))
        return nullptr;

    if (!ppfb->has_symbology())
        return entity;

    T_FaceAttachmentsVec faceAttachmentsVec;

    for (size_t iSymb=0; iSymb < ppfb->symbology()->Length(); iSymb++)
        {
        FB::FaceSymbology const* fbSymb = ((FB::FaceSymbology const*) ppfb->symbology()->Data())+iSymb;
        FaceAttachment attachment;

        if (fbSymb->useColor())
            attachment.SetColor(fbSymb->color(), fbSymb->transparency());

        if (fbSymb->useMaterial())
            attachment.SetMaterial(BeInt64Id((uint64_t)fbSymb->materialId()));

        faceAttachmentsVec.push_back(attachment);
        }

    if (!ppfb->has_symbologyIndex())
        {
        entity->SetFaceMaterialAttachments(faceAttachmentsVec);

        return entity;
        }

    // Support for older BRep that didn't have face attachment index attrib and add the attributes now...
    T_FaceIndexToAttachmentIndexVec faceIndexAttVec;

    for (size_t iSymbIndex=0; iSymbIndex < ppfb->symbologyIndex()->Length(); iSymbIndex++)
        {
        FB::FaceSymbologyIndex const* fbSymbIndex = ((FB::FaceSymbologyIndex const*) ppfb->symbologyIndex()->Data())+iSymbIndex;
        FaceIndexToAttachmentIndex faceAttIndex;

        faceAttIndex.m_faceIndex = fbSymbIndex->faceIndex();
        faceAttIndex.m_symbIndex = fbSymbIndex->symbIndex();

        faceIndexAttVec.push_back(faceAttIndex);
        }

    entity->SetFaceMaterialAttachments(faceAttachmentsVec, &faceIndexAttVec);

    return entity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, DgnGeometryPartId& geomPart, TransformR geomToElem) const
    {
    if (OpCode::GeometryPartInstance != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::GeometryPart>(egOp.m_data);

    geomPart = DgnGeometryPartId((uint64_t)ppfb->geomPartId());

    DPoint3d            origin = (nullptr == ppfb->origin() ? DPoint3d::FromZero() : *((DPoint3dCP) ppfb->origin()));
    YawPitchRollAngles  angles = YawPitchRollAngles::FromDegrees(ppfb->yaw(), ppfb->pitch(), ppfb->roll());
    double              scale  = ppfb->scale();

    geomToElem = angles.ToTransform(origin);

    if (1.0 != scale)
        geomToElem.ScaleMatrixColumns(geomToElem, scale, scale, scale);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, GeometryParamsR elParams) const
    {
    bool changed = false;

    switch (egOp.m_opCode)
        {
        case OpCode::BasicSymbology:
            {
            auto ppfb = flatbuffers::GetRoot<FB::BasicSymbology>(egOp.m_data);

            DgnSubCategoryId subCategoryId((uint64_t)ppfb->subCategoryId());

            if (!subCategoryId.IsValid())
                subCategoryId = elParams.GetSubCategoryId(); // Preserve current sub-category if not explicitly stored (GeometryPart, FaceAttachment, etc.)...

            if (subCategoryId.IsValid())
                {
                DgnCategoryId categoryId = elParams.GetCategoryId(); // Preserve current category and reset to sub-category appearance...

                if (!categoryId.IsValid())
                    categoryId = DgnSubCategory::QueryCategoryId(m_db, subCategoryId);

                elParams = GeometryParams();
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
                DgnStyleId styleId((uint64_t)ppfb->lineStyleId());

                if (elParams.IsLineStyleFromSubCategoryAppearance() || styleId != (nullptr != elParams.GetLineStyle() ? elParams.GetLineStyle()->GetStyleId() : DgnStyleId()))
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
                GradientSymb::Mode  mode = (GradientSymb::Mode) ppfb->mode();

                if (transparency != elParams.GetFillTransparency())
                    {
                    elParams.SetFillTransparency(transparency);
                    changed = true;
                    }

                if (GradientSymb::Mode::None == mode)
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
                    else if (0 != ppfb->backgroundFill())
                        {
                        bool currOutline;
                        bool currBgFill = elParams.IsFillColorFromViewBackground(&currOutline);
                        bool useOutline = (2 == ppfb->backgroundFill());

                        if (!currBgFill || useOutline != currOutline)
                            {
                            elParams.SetFillColorFromViewBackground(useOutline);
                            changed = true;
                            }
                        }
                    }
                else
                    {
                    GradientSymbPtr gradientPtr = GradientSymb::Create();

                    gradientPtr->SetMode(mode);
                    gradientPtr->SetFlags((GradientSymb::Flags)ppfb->flags());
                    gradientPtr->SetShift(ppfb->shift());
                    gradientPtr->SetTint(ppfb->tint());
                    gradientPtr->SetAngle(ppfb->angle());

                    uint32_t nColors = ppfb->colors()->Length();
                    uint32_t const* colors = (uint32_t const*) ppfb->colors()->Data();
                    bvector<ColorDef> keyColors;

                    for (uint32_t iColor=0; iColor < nColors; ++iColor)
                        keyColors.push_back(ColorDef(colors[iColor]));

                    gradientPtr->SetKeys((uint32_t) keyColors.size(), &keyColors.front(), (double const*) ppfb->values()->Data());

                    if (0 != ppfb->thematicSettings())
                        {
                        BeAssert (mode == GradientSymb::Mode::Thematic);
                        auto    thematicSettings = new ThematicGradientSettings();
                        auto    fbThematicSettings = ppfb->thematicSettings();

                        thematicSettings->SetStepCount(fbThematicSettings->stepCount());
                        thematicSettings->SetMarginColor(ColorDef(fbThematicSettings->marginColor()));
                        thematicSettings->SetMode((ThematicGradientSettings::Mode) fbThematicSettings->mode());
                        thematicSettings->SetColorScheme((ThematicGradientSettings::ColorScheme) fbThematicSettings->colorScheme());
                        thematicSettings->SetRange(*((DRange1dCP) fbThematicSettings->range()));
                        gradientPtr->SetThematicSettings(*thematicSettings);
                        }

                    if (nullptr == elParams.GetGradient() || !(*elParams.GetGradient() == *gradientPtr))
                        {
                        elParams.SetGradient(gradientPtr.get());
                        changed = true;
                        }
                    }
                }
            break;
            }

        case OpCode::Pattern:
            {
            auto ppfb = flatbuffers::GetRoot<FB::AreaPattern>(egOp.m_data);
            PatternParamsPtr pattern = PatternParams::Create();

            if (ppfb->has_origin())
                pattern->SetOrigin(*((DPoint3dCP) ppfb->origin()));

            if (ppfb->has_rotation())
                pattern->SetOrientation(*((RotMatrixCP) ppfb->rotation()));

            pattern->SetPrimarySpacing(ppfb->space1());
            pattern->SetSecondarySpacing(ppfb->space2());
            pattern->SetPrimaryAngle(ppfb->angle1());
            pattern->SetSecondaryAngle(ppfb->angle2());
            pattern->SetScale(ppfb->scale());

            if (ppfb->useColor())
                pattern->SetColor(ColorDef(ppfb->color()));

            if (ppfb->useWeight())
                pattern->SetWeight(ppfb->weight());

            pattern->SetInvisibleBoundary(TO_BOOL(ppfb->invisibleBoundary()));
            pattern->SetSnappable(TO_BOOL(ppfb->snappable()));
            pattern->SetSymbolId(DgnGeometryPartId((uint64_t) ppfb->symbolId()));

            if (ppfb->has_defLine())
                {
                flatbuffers::Vector<flatbuffers::Offset<FB::DwgHatchDefLine>> const* fbDefLineOffsets = ppfb->defLine();
                bvector<DwgHatchDefLine> defLines;

                for (auto const& fbDefLine : *fbDefLineOffsets)
                    {
                    DwgHatchDefLine line;

                    line.m_angle   = fbDefLine.angle();
                    line.m_through = *((DPoint2dCP) fbDefLine.through());
                    line.m_offset  = *((DPoint2dCP) fbDefLine.offset());
                    line.m_nDashes = static_cast<short>(fbDefLine.dashes()->Length());

                    if (0 != line.m_nDashes)
                        memcpy(line.m_dashes, fbDefLine.dashes()->Data(), line.m_nDashes * sizeof(double));

                    defLines.push_back(line);
                    }

                pattern->SetDwgHatchDef(defLines);
                }

            if (nullptr == elParams.GetPatternParams() || !(*elParams.GetPatternParams() == *pattern))
                {
                elParams.SetPatternParams(pattern.get());
                changed = true;
                }

            break;
            }

        case OpCode::Material:
            {
            auto ppfb = flatbuffers::GetRoot<FB::Material>(egOp.m_data);

            // NEEDSWORK_WIP_MATERIAL - Set geometry specific material settings of GeometryParams...
            if (ppfb->has_trans2x3())
                {
                Render::MaterialUVDetail detail;
                detail.SetTransform(*reinterpret_cast<Render::TextureMapping::Trans2x3 const*>(ppfb->trans2x3()));
                if (!detail.IsEquivalent(elParams.GetMaterialUVDetail()))
                    {
                    elParams.SetMaterialUVDetail(detail);
                    changed = true;
                    }
                }

            if (ppfb->useMaterial())
                {
                RenderMaterialId material((uint64_t)ppfb->materialId());
                if (elParams.IsMaterialFromSubCategoryAppearance() || material != elParams.GetMaterialId())
                    {
                    elParams.SetMaterialId(material);
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

            DPoint3dCP normal = (DPoint3dCP)ppfb->normal();
            if (nullptr != normal)
                styleParams.normal = *normal;

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, TextStringR text) const
    {
    if (OpCode::TextString != egOp.m_opCode)
        return false;

    return (SUCCESS == TextStringPersistence::DecodeFromFlatBuf(text, egOp.m_data, egOp.m_dataSize));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, ImageGraphicPtr& img) const
    {
    if (OpCode::Image != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::Image>(egOp.m_data);
    if (!ppfb->has_textureId())
        return false;

    ImageGraphic::Corners corners;
    auto* pts = corners.m_pts;
    pts[0] = nullptr != ppfb->tileCorner0() ? *reinterpret_cast<DPoint3dCP>(ppfb->tileCorner0()) : DPoint3d::FromZero();
    pts[1] = nullptr != ppfb->tileCorner1() ? *reinterpret_cast<DPoint3dCP>(ppfb->tileCorner1()) : DPoint3d::FromZero();
    pts[2] = nullptr != ppfb->tileCorner2() ? *reinterpret_cast<DPoint3dCP>(ppfb->tileCorner2()) : DPoint3d::FromZero();
    pts[3] = nullptr != ppfb->tileCorner3() ? *reinterpret_cast<DPoint3dCP>(ppfb->tileCorner3()) : DPoint3d::FromZero();

    DgnTextureId textureId(static_cast<uint64_t>(ppfb->textureId()));
    img = ImageGraphic::Create(corners, textureId, ppfb->drawBorder());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, GeometricPrimitivePtr& elemGeom, bool applyValidation) const
    {
    switch (egOp.m_opCode)
        {
        case GeometryStreamIO::OpCode::PointPrimitive2d:
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
                    elemGeom = GeometricPrimitive::Create(ICurvePrimitive::CreatePointString(&localPoints3dBuf[0], nPts));
                    break;

                case FB::BoundaryType_Open:
                    elemGeom = GeometricPrimitive::Create(ICurvePrimitive::CreateLineString(&localPoints3dBuf[0], nPts));
                    break;

                case FB::BoundaryType_Closed:
                    elemGeom = GeometricPrimitive::Create(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(&localPoints3dBuf[0], nPts)));
                    break;
                }

            return true;
            }

        case GeometryStreamIO::OpCode::PointPrimitive:
            {
            int         nPts;
            int8_t      boundary;
            DPoint3dCP  pts;

            if (!Get(egOp, pts, nPts, boundary))
                break;

            switch (boundary)
                {
                case FB::BoundaryType_None:
                    elemGeom = GeometricPrimitive::Create(ICurvePrimitive::CreatePointString(pts, nPts));
                    break;

                case FB::BoundaryType_Open:
                    elemGeom = GeometricPrimitive::Create(ICurvePrimitive::CreateLineString(pts, nPts));
                    break;

                case FB::BoundaryType_Closed:
                    elemGeom = GeometricPrimitive::Create(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(pts, nPts)));
                    break;
                }

            return true;
            }

        case GeometryStreamIO::OpCode::ArcPrimitive:
            {
            DEllipse3d  arc;
            int8_t      boundary;

            if (!Get(egOp, arc, boundary))
                break;

            switch (boundary)
                {
                case FB::BoundaryType_None:
                case FB::BoundaryType_Open:
                    elemGeom = GeometricPrimitive::Create(ICurvePrimitive::CreateArc(arc));
                    break;

                case FB::BoundaryType_Closed:
                    elemGeom = GeometricPrimitive::Create(CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(arc)));
                    break;
                }

            return true;
            }

        case GeometryStreamIO::OpCode::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePtr;

            if (!Get(egOp, curvePtr, applyValidation))
                break;

            elemGeom = GeometricPrimitive::Create(curvePtr);
            return true;
            }

        case GeometryStreamIO::OpCode::CurveVector:
            {
            CurveVectorPtr curvePtr;

            if (!Get(egOp, curvePtr, applyValidation))
                break;

            elemGeom = GeometricPrimitive::Create(curvePtr);
            return true;
            }

        case GeometryStreamIO::OpCode::Polyface:
            {
            PolyfaceQueryCarrier meshData(0, false, 0, 0, nullptr, nullptr);

            if (!Get(egOp, meshData, applyValidation))
                break;

            elemGeom = GeometricPrimitive::Create(meshData);
            return true;
            }

        case GeometryStreamIO::OpCode::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPtr;

            if (!Get(egOp, solidPtr, applyValidation))
                break;

            elemGeom = GeometricPrimitive::Create(solidPtr);
            return true;
            }

        case GeometryStreamIO::OpCode::BsplineSurface:
            {
            MSBsplineSurfacePtr surfacePtr;

            if (!Get(egOp, surfacePtr, applyValidation))
                break;

            elemGeom = GeometricPrimitive::Create(surfacePtr);
            return true;
            }

        case GeometryStreamIO::OpCode::ParasolidBRep:
            {
            IBRepEntityPtr entityPtr;

            if (!Get(egOp, entityPtr))
                break;

            elemGeom = GeometricPrimitive::Create(entityPtr);
            return true;
            }

        case GeometryStreamIO::OpCode::TextString:
            {
            TextStringPtr text = TextString::Create(m_db);
            if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(*text, egOp.m_data, egOp.m_dataSize))
                break;

            elemGeom = GeometricPrimitive::Create(text);
            return true;
            }

        case GeometryStreamIO::OpCode::Image:
            {
            ImageGraphicPtr img;
            if (!Get(egOp, img))
                break;

            elemGeom = GeometricPrimitive::Create(img);
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Iterator::ToNext()
    {
    if (m_dataOffset >= m_dataSize)
        {
        m_data = nullptr;
        m_dataOffset = 0;

        return;
        }

    uint32_t        opCode = *((uint32_t const*) (m_data));
    uint32_t        dataSize = *((uint32_t const*) (m_data + sizeof (opCode)));
    uint8_t const*  data = (0 != dataSize ? (uint8_t const*) (m_data + sizeof (opCode) + sizeof (dataSize)) : nullptr);
    size_t          egOpSize = sizeof (opCode) + sizeof (dataSize) + dataSize;

    m_egOp = Operation((OpCode) (opCode), dataSize, data);
    m_data += egOpSize;
    m_dataOffset += egOpSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStreamIO::Import(GeometryStreamR dest, GeometryStreamCR source, DgnImportContext& importer)
    {
    if (!source.HasGeometry())
        return DgnDbStatus::Success; // otherwise we end up writing a header for an otherwise empty stream...

    Writer writer(importer.GetDestinationDb());
    Collection collection(source.GetData(), source.GetSize());
    writer.Reset(collection.GetHeader().m_flags);

    bool isFilteringBySubCategory = false;

    for (auto const& egOp : collection)
        {
        if ((GeometryStreamIO::OpCode::BasicSymbology == egOp.m_opCode) && importer.HasSubCategoryFilter())
            {
            DgnSubCategoryId subCategoryId((uint64_t)flatbuffers::GetRoot<FB::BasicSymbology>(egOp.m_data)->subCategoryId());
            isFilteringBySubCategory = subCategoryId.IsValid() && importer.IsSubCategoryFiltered(subCategoryId); // !IsValid indicates the default SubCategory which is never filtered
            }

        if (isFilteringBySubCategory)
            continue; // A filtered SubCategory is still active, so skip the current entry. Note this may leave the bounding box bigger than needed but decided it wasn't worth the trouble to update.

        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                continue; // Appended by Reset() above.
            case GeometryStreamIO::OpCode::BasicSymbology:
                {
                auto ppfb = flatbuffers::GetRoot<FB::BasicSymbology>(egOp.m_data);

                DgnSubCategoryId subCategoryId((uint64_t)ppfb->subCategoryId());
                DgnSubCategoryId remappedSubCategoryId = (subCategoryId.IsValid() ? importer.FindSubCategory(subCategoryId) : DgnSubCategoryId());
                BeAssert((subCategoryId.IsValid() == remappedSubCategoryId.IsValid()) && "Category and all subcategories should have been remapped by the element that owns this geometry");

                DgnStyleId lineStyleId((uint64_t)ppfb->lineStyleId());
                DgnStyleId remappedLineStyleId = (lineStyleId.IsValid() ? importer.RemapLineStyleId(lineStyleId) : DgnStyleId());
                //BeAssert((lineStyleId.IsValid() == remappedLineStyleId.IsValid()));

                FlatBufferBuilder remappedfbb;

                auto mloc = FB::CreateBasicSymbology(remappedfbb, remappedSubCategoryId.GetValueUnchecked(),
                                                     ppfb->color(), ppfb->weight(), remappedLineStyleId.GetValueUnchecked(),
                                                     ppfb->transparency(), ppfb->displayPriority(), ppfb->geomClass(),
                                                     ppfb->useColor(), ppfb->useWeight(), ppfb->useStyle());
                remappedfbb.Finish(mloc);
                writer.Append(Operation(OpCode::BasicSymbology, (uint32_t) remappedfbb.GetSize(), remappedfbb.GetBufferPointer()));
                break;
                }

            case GeometryStreamIO::OpCode::GeometryPartInstance:
                {
                auto ppfb = flatbuffers::GetRoot<FB::GeometryPart>(egOp.m_data);

                DgnGeometryPartId remappedGeometryPartId = importer.RemapGeometryPartId(DgnGeometryPartId((uint64_t) ppfb->geomPartId())); // Trigger deep-copy if necessary
                BeAssert(remappedGeometryPartId.IsValid() && "Unable to deep-copy geompart!");

                FlatBufferBuilder remappedfbb;

                auto mloc = FB::CreateGeometryPart(remappedfbb, remappedGeometryPartId.GetValueUnchecked(), ppfb->origin(), ppfb->yaw(), ppfb->pitch(), ppfb->roll(), ppfb->scale());

                remappedfbb.Finish(mloc);
                writer.Append(Operation(OpCode::GeometryPartInstance, (uint32_t) remappedfbb.GetSize(), remappedfbb.GetBufferPointer()));
                break;
                }

            case OpCode::Pattern:
                {
                auto ppfb = flatbuffers::GetRoot<FB::AreaPattern>(egOp.m_data);

                if (!ppfb->has_symbolId())
                    {
                    writer.Append(egOp);
                    break;
                    }

                DgnGeometryPartId remappedGeometryPartId = importer.RemapGeometryPartId(DgnGeometryPartId((uint64_t) ppfb->symbolId())); // Trigger deep-copy if necessary
                BeAssert(remappedGeometryPartId.IsValid() && "Unable to deep-copy geompart!");

                FlatBufferBuilder remappedfbb;

                auto mloc = FB::CreateAreaPattern(remappedfbb, ppfb->origin(), ppfb->rotation(), ppfb->space1(), ppfb->space2(), ppfb->angle1(), ppfb->angle2(), ppfb->scale(),
                                                  ppfb->color(), ppfb->weight(), ppfb->useColor(), ppfb->useWeight(), ppfb->invisibleBoundary(), ppfb->snappable(),
                                                  remappedGeometryPartId.GetValueUnchecked());
                remappedfbb.Finish(mloc);
                writer.Append(Operation(OpCode::Pattern, (uint32_t) remappedfbb.GetSize(), remappedfbb.GetBufferPointer()));
                break;
                }

            case GeometryStreamIO::OpCode::Material:
                {
                auto fbSymb = flatbuffers::GetRoot<FB::Material>(egOp.m_data);
                RenderMaterialId materialId((uint64_t)fbSymb->materialId());
                RenderMaterialId remappedMaterialId = (materialId.IsValid() ? importer.RemapRenderMaterialId(materialId) : RenderMaterialId());
                BeAssert((materialId.IsValid() == remappedMaterialId.IsValid()) && "Unable to deep-copy material");

                FlatBufferBuilder remappedfbb;
                auto mloc = FB::CreateMaterial(remappedfbb, fbSymb->useMaterial(), remappedMaterialId.GetValueUnchecked(), fbSymb->origin(), fbSymb->size(), fbSymb->yaw(), fbSymb->pitch(), fbSymb->roll(), fbSymb->trans2x3());
                remappedfbb.Finish(mloc);
                writer.Append(Operation(OpCode::Material, (uint32_t) remappedfbb.GetSize(), remappedfbb.GetBufferPointer()));
                break;
                }

            case GeometryStreamIO::OpCode::ParasolidBRep:
                {
                auto ppfb = flatbuffers::GetRoot<FB::BRepData>(egOp.m_data);

                if (!ppfb->has_symbology())
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
                        RenderMaterialId materialId((uint64_t)fbSymb->materialId());
                        RenderMaterialId remappedMaterialId = (materialId.IsValid() ? importer.RemapRenderMaterialId(materialId) : RenderMaterialId());
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
                auto remappedFaceSymbIndex = ppfb->has_symbologyIndex() ? remappedfbb.CreateVectorOfStructs((FB::FaceSymbologyIndex const*) ppfb->symbologyIndex()->Data(), ppfb->symbologyIndex()->Length()) : 0;
                auto mloc = FB::CreateBRepData(remappedfbb, ppfb->entityTransform(), ppfb->brepType(), remappedEntityData, remappedFaceSymb, remappedFaceSymbIndex);
                remappedfbb.Finish(mloc);
                writer.Append(Operation(OpCode::ParasolidBRep, (uint32_t) remappedfbb.GetSize(), remappedfbb.GetBufferPointer()));
                break;
                }

            case GeometryStreamIO::OpCode::TextString:
                {
                TextStringPtr text = TextString::Create(importer.GetSourceDb());
                if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(*text, egOp.m_data, egOp.m_dataSize))
                    break;

                FontId srcFontId = text->GetStyle().GetFont();
                FontId dstFontId = importer.RemapFont(srcFontId);
                text->ChangeDgnDb(&importer.GetDestinationDb());
                text->GetStyleR().SetFont(dstFontId);

                writer.Append(*text);
                break;
                }

            case GeometryStreamIO::OpCode::Image:
                {
                auto fbImg = flatbuffers::GetRoot<FB::Image>(egOp.m_data);
                if (!fbImg->has_textureId())
                    {
                    writer.Append(egOp);
                    break;
                    }

                DgnTextureId srcTextureId(static_cast<uint64_t>(fbImg->textureId()));
                DgnTextureId dstTextureId = srcTextureId.IsValid() ? importer.RemapTextureId(srcTextureId) : DgnTextureId();
                BeAssert(srcTextureId.IsValid() == dstTextureId.IsValid() && "Unable to deep-copy texture for ImageGraphic");

                FlatBufferBuilder fbb;
                auto mloc = FB::CreateImage(
                    fbb,
                    fbImg->drawBorder() ? 1 : 0,
                    0, // use fill tint
                    fbImg->tileCorner0(), fbImg->tileCorner1(), fbImg->tileCorner2(), fbImg->tileCorner3(),
                    0, 0, 0, // width, height, format
                    0, // image bytes
                    static_cast<int64_t>(dstTextureId.GetValueUnchecked()));

                fbb.Finish(mloc);
                writer.Append(Operation(OpCode::Image, static_cast<uint32_t>(fbb.GetSize()), fbb.GetBufferPointer()));
                break;
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElementUsageInfo::ScanGeometryStream(GeometryStreamCR geometryStream)
    {
    GeometryStreamIO::Collection geometryCollection(geometryStream.GetData(), geometryStream.GetSize());
    for (auto const& entry : geometryCollection)
        {
        switch (entry.m_opCode)
            {
            case GeometryStreamIO::OpCode::BasicSymbology:
                if (!m_subCategoryIds.empty() || !m_lineStyleIds.empty())
                    {
                    auto basicSymbologyFB = flatbuffers::GetRoot<FB::BasicSymbology>(entry.m_data);
                    DgnSubCategoryId subCategoryId(static_cast<uint64_t>(basicSymbologyFB->subCategoryId()));
                    if (subCategoryId.IsValid() && m_subCategoryIds.Contains(subCategoryId))
                        m_usedIds.insert(subCategoryId);

                    DgnStyleId lineStyleId(static_cast<uint64_t>(basicSymbologyFB->lineStyleId()));
                    if (lineStyleId.IsValid() && m_lineStyleIds.Contains(lineStyleId))
                        m_usedIds.insert(lineStyleId);
                    }
                break;

            case GeometryStreamIO::OpCode::GeometryPartInstance:
                if (!m_geometryPartIds.empty())
                    {
                    auto geometryPartFB = flatbuffers::GetRoot<FB::GeometryPart>(entry.m_data);
                    DgnGeometryPartId geometryPartId(static_cast<uint64_t>(geometryPartFB->geomPartId()));
                    if (geometryPartId.IsValid() && m_geometryPartIds.Contains(geometryPartId))
                        m_usedIds.insert(geometryPartId);
                    }
                break;

            case GeometryStreamIO::OpCode::Pattern:
                if (!m_geometryPartIds.empty())
                    {
                    auto areaPatternFB = flatbuffers::GetRoot<FB::AreaPattern>(entry.m_data);
                    if (areaPatternFB->has_symbolId())
                        {
                        DgnGeometryPartId geometryPartId(static_cast<uint64_t>(areaPatternFB->symbolId()));
                        if (geometryPartId.IsValid() && m_geometryPartIds.Contains(geometryPartId))
                            m_usedIds.insert(geometryPartId);
                        }
                    }
                break;

            case GeometryStreamIO::OpCode::Material:
                if (!m_renderMaterialIds.empty())
                    {
                    auto materialFB = flatbuffers::GetRoot<FB::Material>(entry.m_data);
                    RenderMaterialId renderMaterialId(static_cast<uint64_t>(materialFB->materialId()));
                    if (renderMaterialId.IsValid() && m_renderMaterialIds.Contains(renderMaterialId))
                        m_usedIds.insert(renderMaterialId);
                    }
                break;

            case GeometryStreamIO::OpCode::ParasolidBRep:
                if (!m_renderMaterialIds.empty())
                    {
                    auto brepFB = flatbuffers::GetRoot<FB::BRepData>(entry.m_data);
                    if (brepFB->has_symbology())
                        {
                        for (size_t iSymb=0; iSymb < brepFB->symbology()->Length(); iSymb++)
                            {
                            FB::FaceSymbology const* faceSymbologyFB = ((FB::FaceSymbology const*) brepFB->symbology()->Data())+iSymb;

                            if (faceSymbologyFB->useMaterial())
                                {
                                RenderMaterialId renderMaterialId(static_cast<uint64_t>(faceSymbologyFB->materialId()));
                                if (renderMaterialId.IsValid() && m_renderMaterialIds.Contains(renderMaterialId))
                                    m_usedIds.insert(renderMaterialId);
                                }
                            }
                        }
                    }
                break;

            case GeometryStreamIO::OpCode::Image:
                if (!m_textureIds.empty())
                    {
                    auto imageFB = flatbuffers::GetRoot<FB::Image>(entry.m_data);
                    if (imageFB->has_textureId())
                        {
                        DgnTextureId textureId(static_cast<uint64_t>(imageFB->textureId()));
                        if (textureId.IsValid() && m_textureIds.Contains(textureId))
                            m_usedIds.insert(textureId);
                        }
                    }
                break;

            default:
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStreamIO::ConvertBRepsToPolyfacesOrCurves(DgnDbR db, GeometryStreamR output, GeometryStreamCR input, bool& changed)
    {
    if (!input.HasGeometry())
        return DgnDbStatus::NoGeometry;

    Reader reader(db);
    Writer writer(db);
    Collection collection(input.GetData(), input.GetSize());
    writer.Reset(collection.GetHeader().m_flags);

    // 0.2 angle tolerance is what we previously used for pre-meshing data on platforms where Parasolid was unavailable.
    auto facetOptions = IFacetOptions::CreateForSurfaces(0.0, 0.2, 0.0, true, true, true);
    facetOptions->SetIgnoreHiddenBRepEntities(true); // big perf improvement, hidden BREP entities are unused anyway
    facetOptions->SetBRepConcurrentFacetting(false); // see Parasolid IR 8415939, always faster exclusive

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                break; // Appended by Reset() above.

            case GeometryStreamIO::OpCode::ParasolidBRep:
                {
                writer.AppendNonBRepRepresentation(egOp, facetOptions.get());
                changed = true;
                break;
                }

            default:
                {
                writer.Append(egOp);
                break;
                }
            }
        }

    if (!changed)
        return DgnDbStatus::Success;

    output.Clear();
    output.SaveData(&writer.m_buffer.front(), (uint32_t) writer.m_buffer.size());

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static IFacetOptionsPtr createFacetOptions(Napi::Object const& requestProps)
    {
    Napi::Value chordTolVal = requestProps.Get("chordTol");
    Napi::Value angleTolVal = requestProps.Get("angleTol");
    Napi::Value maxEdgeLengthVal = requestProps.Get("maxEdgeLength");
    Napi::Value minBRepFeatureSize = requestProps.Get("minBRepFeatureSize");

    if (!chordTolVal.IsNumber() && !angleTolVal.IsNumber() && !maxEdgeLengthVal.IsNumber() && !minBRepFeatureSize.IsNumber())
        return nullptr;

    double chordTol = chordTolVal.IsNumber() ? chordTolVal.As<Napi::Number>().DoubleValue() : 0.0;
    double angleTol = angleTolVal.IsNumber() ? std::min(msGeomConst_piOver4, angleTolVal.As<Napi::Number>().DoubleValue()) : msGeomConst_piOver12;
    double maxEdgeLength = maxEdgeLengthVal.IsNumber() ? maxEdgeLengthVal.As<Napi::Number>().DoubleValue() : 0.0;

    auto result = IFacetOptions::CreateForSurfaces(chordTol, angleTol, maxEdgeLength, true, true, true);
    result->SetIgnoreHiddenBRepEntities(true); // act like tile generation, big perf improvement
    result->SetBRepConcurrentFacetting(false); // see Parasolid IR 8415939, always faster exclusive

    if (minBRepFeatureSize.IsNumber())
        result->SetBRepIgnoredFeatureSize(minBRepFeatureSize.As<Napi::Number>().DoubleValue());

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryStreamIO::Operation GeometryStreamIO::ToOperation(Napi::Value const& entryVal)
    {
    if (!entryVal.IsObject())
        return GeometryStreamIO::Operation();

    Napi::Object entryObj = entryVal.As<Napi::Object>();
    Napi::Number opcodeVal = entryObj.Get("opcode").As<Napi::Number>();
    if (!opcodeVal.IsNumber())
        return GeometryStreamIO::Operation();

    Napi::TypedArray dataVal = entryObj.Get("data").As<Napi::TypedArray>();
    if (!dataVal.IsTypedArray() || napi_uint8_array != dataVal.TypedArrayType())
        return GeometryStreamIO::Operation();

    uint32_t opcode = opcodeVal.As<Napi::Number>().Uint32Value();
    Napi::Uint8Array uint8ArrayVal = dataVal.As<Napi::Uint8Array>();
    return GeometryStreamIO::Operation((GeometryStreamIO::OpCode) opcode, (uint32_t) uint8ArrayVal.ByteLength(), uint8ArrayVal.Data());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object GeometryStreamIO::FromOperation(GeometryStreamIO::Operation const& egOp, Napi::Env env)
    {
    Napi::Uint8Array dataArray = Napi::Uint8Array::New(env, egOp.m_dataSize);
    memcpy(dataArray.Data(), egOp.m_data, egOp.m_dataSize);

    Napi::Object cbEntry = Napi::Object::New(env);
    cbEntry.Set("opcode", Napi::Number::New(env, (uint32_t) egOp.m_opCode));
    cbEntry.Set("data", dataArray);

    return cbEntry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GeometryStreamIO::ToGeometricPrimitive(DgnDbR db, Napi::Object const& opProps)
    {
    GeometryStreamIO::Operation egOp = ToOperation(opProps);

    if (!egOp.IsGeometryOp())
        return nullptr;

    Reader reader(db);
    GeometricPrimitivePtr geom;

    if (!reader.Get(egOp, geom, true))
        return nullptr;

    return geom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStreamIO::ProcessGeometryStream(DgnElementCR element, Napi::Object const& requestProps, Napi::Env env)
    {
    GeometrySourceCP source = element.ToGeometrySource();
    DgnGeometryPartCP part = (nullptr == source ? element.ToGeometryPart() : nullptr);
    if (nullptr == source && nullptr == part)
        return DgnDbStatus::BadElement;

    Napi::Function onGeometryCb = requestProps.Get("onGeometry").As<Napi::Function>();
    Napi::Boolean skipBRepsVal = requestProps.Get("skipBReps").As<Napi::Boolean>();
    Napi::Boolean replaceBRepsVal = requestProps.Get("replaceBReps").As<Napi::Boolean>();
    bool skipBReps = skipBRepsVal.IsBoolean() ? skipBRepsVal.Value() : false;
    bool replaceBReps = replaceBRepsVal.IsBoolean() ? replaceBRepsVal.Value() : false;
    IFacetOptionsPtr facetOptions = replaceBReps ? createFacetOptions(requestProps) : nullptr;

    DPoint3d bboxPts[2];
    GeometryStreamCR stream = (nullptr != source ? source->GetGeometryStream() : part->GetGeometryStream());
    GeometryStreamIO::Collection collection(stream.GetData(), stream.GetSize());

    Napi::Object cbArgument = Napi::Object::New(env);

    if (nullptr != source)
        {
        static Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        source->GetCategoryId().ToString(idStrBuffer, BeInt64Id::UseHex::Yes);
        cbArgument.Set("categoryId", Napi::String::New(env, idStrBuffer));

        Napi::Float64Array transformArray = Napi::Float64Array::New(env, 12);
        memcpy(transformArray.Data(), source->GetPlacementTransform().form3d, 12 * sizeof(double));
        cbArgument.Set("sourceToWorld", transformArray);

        GeometrySource3dCP source3d = element.ToGeometrySource3d();
        GeometrySource2dCP source2d = (nullptr == source3d ? element.ToGeometrySource2d() : nullptr);
        if (nullptr != source3d)
            {
            ElementAlignedBox3d bbox = source3d->GetPlacement().GetElementBox();

            bboxPts[0] = bbox.low;
            bboxPts[1] = bbox.high;
            }
        else if (nullptr != source2d)
            {
            ElementAlignedBox2d bbox = source2d->GetPlacement().GetElementBox();

            bboxPts[0] = DPoint3d::From(bbox.low);
            bboxPts[1] = DPoint3d::From(bbox.high);
            }
        }
    else
        {
        ElementAlignedBox3d bbox = part->GetBoundingBox();

        bboxPts[0] = bbox.low;
        bboxPts[1] = bbox.high;
        }

    Napi::Float64Array bboxArray = Napi::Float64Array::New(env, 6);
    memcpy(bboxArray.Data(), bboxPts, 6 * sizeof(double));
    cbArgument.Set("bbox", bboxArray);

    if (GeometryStreamIO::Header::Flags::None != (collection.GetHeader().m_flags & GeometryStreamIO::Header::Flags::ViewIndependent))
        cbArgument.Set("viewIndependent", Napi::Boolean::New(env, true));

    Napi::Array cbData = Napi::Array::New(env);
    uint32_t index = 0;
    bool hasBRep = false;

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                break;

            case GeometryStreamIO::OpCode::ParasolidBRep:
                {
                hasBRep = true;

                if (skipBReps)
                    break;

                if (replaceBReps)
                    {
                    GeometryStreamIO::Writer writer(element.GetDgnDb());

                    if (writer.AppendNonBRepRepresentation(egOp, facetOptions.IsValid() ? facetOptions.get() : nullptr))
                        {
                        GeometryStreamIO::Collection brepCollection(&writer.m_buffer.front(), writer.m_buffer.size());

                        for (auto const& brepEgOp : brepCollection)
                            {
                            if (GeometryStreamIO::OpCode::Header == brepEgOp.m_opCode)
                                continue;

                            Napi::Object cbEntry = FromOperation(brepEgOp, env);
                            cbData.Set(index++, cbEntry);
                            }
                        }
                    break;
                    }

                // FALL THROUGH...
                }

            default:
                {
                Napi::Object cbEntry = FromOperation(egOp, env);
                cbData.Set(index++, cbEntry);
                break;
                }
            }
        }

    if (hasBRep && (skipBReps || replaceBReps))
        cbArgument.Set("brepsPresent", Napi::Boolean::New(env, true));

    cbArgument.Set("entryArray", cbData);
    onGeometryCb.Call({ cbArgument });

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStreamIO::ProcessGeometryStream(DgnDbR db, Napi::Object const& requestProps, Napi::Env env)
    {
    Napi::String napiElementId = requestProps.Get("elementId").As<Napi::String>();
    std::string elementIdStr = napiElementId.Utf8Value();
    DgnElementId elementId(BeInt64Id::FromString(elementIdStr.c_str()).GetValue());
    if (!elementId.IsValid())
        return DgnDbStatus::InvalidId;

    DgnElementCPtr element = db.Elements().GetElement(elementId);
    if (!element.IsValid())
        return DgnDbStatus::InvalidId;

    return ProcessGeometryStream(*element, requestProps, env);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStreamIO::BuildGeometryStream(DgnElementR element, GeometryBuilderParams const& bParams, Napi::Array entryArrayObj)
    {
    auto& db = element.GetDgnDb();

    GeometrySourceP source = element.ToGeometrySourceP();
    DgnGeometryPartP part = (nullptr == source ? element.ToGeometryPartP() : nullptr);
    if (nullptr == source && nullptr == part)
        return DgnDbStatus::BadElement;

    GeometryBuilderPtr builder = (nullptr != source ? GeometryBuilder::Create(*source) : GeometryBuilder::CreateGeometryPart(db, !bParams.is2dPart));
    if (!builder.IsValid())
        return DgnDbStatus::BadElement;

    if (bParams.viewIndependent)
        builder->SetHeaderFlags(GeometryStreamIO::Header::Flags::ViewIndependent);

    // A zero length array means clear the geometry stream and invalidate element aligned box...
    if (0 == entryArrayObj.Length() && nullptr != source)
        return (SUCCESS == builder->ClearGeometryStream(*source) ? DgnDbStatus::Success : DgnDbStatus::NoGeometry);

    GeometryBuilder::CoordSystem coordSys = bParams.isWorld ? GeometryBuilder::CoordSystem::World : GeometryBuilder::CoordSystem::Local;

    Render::GeometryParams geomParams = builder->GetGeometryParams();
    Reader reader(db);

    for (uint32_t index = 0, nEntries = entryArrayObj.Length(); index < nEntries; ++index)
        {
        GeometryStreamIO::Operation egOp = ToOperation(entryArrayObj.Get(index));

        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                break; // Ignore if present...

            case GeometryStreamIO::OpCode::BasicSymbology:
            case GeometryStreamIO::OpCode::LineStyleModifiers:
            case GeometryStreamIO::OpCode::AreaFill:
            case GeometryStreamIO::OpCode::Pattern:
            case GeometryStreamIO::OpCode::Material:
                {
                if (!reader.Get(egOp, geomParams))
                    return DgnDbStatus::BadArg;

                if (!builder->Append(geomParams, coordSys))
                    return DgnDbStatus::BadArg;
                break;
                }

            case GeometryStreamIO::OpCode::SubGraphicRange:
                {
                bool hasMultipleGeom = false;

                // Don't append sub-graphic ranges unless multiple geometric primitives remain to be added...
                if (index < nEntries - 2)
                    {
                    for (uint32_t next = index + 1, nGeom = 0; next < nEntries; ++next)
                        {
                        GeometryStreamIO::Operation nextEgOp = ToOperation(entryArrayObj.Get(next));
                        if (nextEgOp.IsGeometryOp() || GeometryStreamIO::OpCode::GeometryPartInstance == nextEgOp.m_opCode)
                            nGeom++; // For counting purposes, a GeometryPart is considered geometry (i.e. stream includes mix of part instances and primitives)...

                        if (nGeom < 2)
                            continue;

                        hasMultipleGeom = true;
                        break;
                        }
                    }

                if (hasMultipleGeom)
                    builder->SetAppendAsSubGraphics(); // This will do nothing if a GeometryPart is being created, or a GeometryPart instance is being inserted...
                break;
                }

            case GeometryStreamIO::OpCode::GeometryPartInstance:
                {
                DgnGeometryPartId geomPartId;
                Transform         geomToSource;

                if (!reader.Get(egOp, geomPartId, geomToSource))
                    return DgnDbStatus::BadArg;

                if (!builder->Append(geomPartId, geomToSource))
                    return DgnDbStatus::BadArg;
                break;
                }

            default:
                {
                if (!egOp.IsGeometryOp())
                    return DgnDbStatus::BadArg;

                GeometricPrimitivePtr geom;
                if (!reader.Get(egOp, geom, true))
                    return DgnDbStatus::BadArg;

                if (!builder->Append(*geom, coordSys))
                    return DgnDbStatus::BadArg;
                break;
                }
            }
        }

    if (nullptr != source)
        {
        if (SUCCESS != builder->Finish(*source))
            return DgnDbStatus::NoGeometry;
        }
    else
        {
        if (SUCCESS != builder->Finish(*part))
            return DgnDbStatus::NoGeometry;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStreamIO::CreateBRepGeometry(DgnDbR db, Napi::Object const& createProps, Napi::Env env)
    {
    return T_HOST.GetBRepGeometryAdmin()._CreateBRepGeometry(db, createProps, env);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Collection::HasSymbologyChanges(Render::GeometryParamsCR baseParams) const
    {
    auto const& geom = *this;
    for (auto const& entry : geom)
        {
        switch (entry.m_opCode)
            {
            case GeometryStreamIO::OpCode::BasicSymbology:
            case GeometryStreamIO::OpCode::AreaFill:
            case GeometryStreamIO::OpCode::Pattern:
            case GeometryStreamIO::OpCode::Material:
            case GeometryStreamIO::OpCode::LineStyleModifiers:
                return true;

            case GeometryStreamIO::OpCode::PointPrimitive:
            case GeometryStreamIO::OpCode::PointPrimitive2d:
            case GeometryStreamIO::OpCode::ArcPrimitive:
            case GeometryStreamIO::OpCode::CurveVector:
            case GeometryStreamIO::OpCode::CurvePrimitive:
                // Don't instance part that may inherit linestyle from geometric element...
                if (baseParams.HasStrokedLineStyle())
                    return true;

                break;

            case GeometryStreamIO::OpCode::ParasolidBRep:
                {
                // Don't instance BReps with face-attached materials.
                auto ppfb = flatbuffers::GetRoot<FB::BRepData>(entry.m_data);
                if (ppfb->has_symbology() && ppfb->symbology()->Length() > 0)
                    return true;

                break;
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void debugGeomId(GeometryStreamIO::IDebugOutput& output, GeometricPrimitiveCR geom, GeometryStreamEntryId geomId)
    {
    Utf8String  geomType;

    switch (geom.GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurvePrimitive:
            geomType.assign("CurvePrimitive");
            break;

        case GeometricPrimitive::GeometryType::CurveVector:
            geomType.assign("CurveVector");
            break;

        case GeometricPrimitive::GeometryType::SolidPrimitive:
            geomType.assign("SolidPrimitive");
            break;

        case GeometricPrimitive::GeometryType::BsplineSurface:
            geomType.assign("BsplineSurface");
            break;

        case GeometricPrimitive::GeometryType::Polyface:
            geomType.assign("Polyface");
            break;

        case GeometricPrimitive::GeometryType::BRepEntity:
            geomType.assign("BRepEntity");
            break;

        case GeometricPrimitive::GeometryType::TextString:
            geomType.assign("TextString");
            break;

        case GeometricPrimitive::GeometryType::Image:
            geomType.assign("Image");
            break;

        default:
            geomType.assign("Unknown");
            break;
        }

    if (!geomId.GetGeometryPartId().IsValid())
        output._DoOutputLine(Utf8PrintfString("- GeometryType::%s \t[Index: %d]\n", geomType.c_str(), geomId.GetIndex()).c_str());
    else
        output._DoOutputLine(Utf8PrintfString("- GeometryType::%s \t[Index: %d | PartId: %" PRIu64 " Part Index: %d]\n", geomType.c_str(), geomId.GetIndex(), geomId.GetGeometryPartId().GetValue(), geomId.GetPartIndex()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Debug(IDebugOutput& output, GeometryStreamCR stream, DgnDbR db, bool isPart)
    {
// #define DEBUG_POLYFACE_POINT_COUNTS
    Collection  collection(stream.GetData(), stream.GetSize());
    Reader      reader(db);

    IdSet<DgnGeometryPartId> parts;

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                {
                output._DoOutputLine("OpCode::Header\n");
                break;
                }

            case GeometryStreamIO::OpCode::SubGraphicRange:
                {
                output._DoOutputLine("OpCode::SubGraphicRange\n");
                break;
                }

            case GeometryStreamIO::OpCode::GeometryPartInstance:
                {
                auto ppfb = flatbuffers::GetRoot<FB::GeometryPart>(egOp.m_data);

                DgnGeometryPartId   partId = DgnGeometryPartId((uint64_t)ppfb->geomPartId());
                DPoint3d            origin = (nullptr == ppfb->origin() ? DPoint3d::FromZero() : *((DPoint3dCP) ppfb->origin()));
                YawPitchRollAngles  angles = YawPitchRollAngles::FromDegrees(ppfb->yaw(), ppfb->pitch(), ppfb->roll());

                if (output._WantPartGeometry())
                    parts.insert(partId);

                output._DoOutputLine(Utf8PrintfString("OpCode::GeometryPartInstance - PartId: %" PRIu64 "\n", partId.GetValue()).c_str());

                if (!output._WantVerbose())
                    break;

                // Transform geomToElem = angles.ToTransform(origin);
                //
                // for (int i=0; i<3; i++)
                //     output._DoOutputLine(Utf8PrintfString("  [%lf, \t%lf, \t%lf, \t%lf]\n", geomToElem.form3d[i][0], geomToElem.form3d[i][1], geomToElem.form3d[i][2], geomToElem.form3d[i][3]).c_str());

                if (!(ppfb->has_origin() || ppfb->has_yaw() || ppfb->has_pitch() || ppfb->has_roll() || ppfb->has_scale()))
                    break;

                output._DoOutputLine("  ");

                if (ppfb->has_origin())
                    output._DoOutputLine(Utf8PrintfString("Origin: [%lf, %lf, %lf] ", origin.x, origin.y, origin.z).c_str());

                if (ppfb->has_yaw())
                    output._DoOutputLine(Utf8PrintfString("Yaw: %lf ", angles.GetYaw().Degrees()).c_str());

                if (ppfb->has_pitch())
                    output._DoOutputLine(Utf8PrintfString("Pitch: %lf ", angles.GetPitch().Degrees()).c_str());

                if (ppfb->has_roll())
                    output._DoOutputLine(Utf8PrintfString("Roll: %lf ", angles.GetRoll().Degrees()).c_str());

                if (ppfb->has_scale())
                    output._DoOutputLine(Utf8PrintfString("Scale: %lf ", ppfb->scale()).c_str());

                output._DoOutputLine("\n");
                break;
                }

            case GeometryStreamIO::OpCode::BasicSymbology:
                {
                auto ppfb = flatbuffers::GetRoot<FB::BasicSymbology>(egOp.m_data);

                DgnSubCategoryId   subCategoryId((uint64_t)ppfb->subCategoryId());
                DgnSubCategoryCPtr subCat = (subCategoryId.IsValid() ? DgnSubCategory::Get(db, subCategoryId) : nullptr);

                if (subCat.IsValid() && nullptr != subCat->GetCode().GetValueUtf8CP())
                    output._DoOutputLine(Utf8PrintfString("OpCode::BasicSymbology - SubCategory: %s - Id: %" PRIu64 "\n", subCat->GetCode().GetValueUtf8CP(), subCategoryId.GetValue()).c_str());
                else if (subCategoryId.IsValid())
                    output._DoOutputLine(Utf8PrintfString("OpCode::BasicSymbology - SubCategoryId: %" PRIu64 "\n", subCategoryId.GetValue()).c_str());
                else
                    output._DoOutputLine("OpCode::BasicSymbology\n");

                if (!output._WantVerbose())
                    break;

                if (!(ppfb->has_color() || ppfb->has_useColor() ||
                      ppfb->has_weight() || ppfb->has_useWeight() ||
                      ppfb->has_lineStyleId() || ppfb->has_useStyle() ||
                      ppfb->has_transparency() || ppfb->has_displayPriority() || ppfb->has_geomClass()))
                    break;

                output._DoOutputLine("  ");

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

                output._DoOutputLine("\n");
                break;
                }

            case GeometryStreamIO::OpCode::PointPrimitive:
                {
                output._DoOutputLine("OpCode::PointPrimitive\n");

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

            case GeometryStreamIO::OpCode::PointPrimitive2d:
                {
                output._DoOutputLine("OpCode::PointPrimitive2d\n");

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

            case GeometryStreamIO::OpCode::ArcPrimitive:
                {
                output._DoOutputLine("OpCode::ArcPrimitive\n");

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

            case GeometryStreamIO::OpCode::CurveVector:
                {
                output._DoOutputLine("OpCode::CurveVector\n");
                break;
                }

            case GeometryStreamIO::OpCode::Polyface:
                {
#if defined(DEBUG_POLYFACE_POINT_COUNTS)
                PolyfaceHeaderPtr mesh = BentleyGeometryFlatBuffer::BytesToPolyfaceHeader(egOp.m_data);
                output._DoOutputLine(Utf8PrintfString("OpCode::Polyface %u points\n", mesh.IsValid() ? static_cast<uint32_t>(mesh->GetPointCount()) : 0).c_str());
#else
                output._DoOutputLine("OpCode::Polyface\n");
#endif
                break;
                }

            case GeometryStreamIO::OpCode::CurvePrimitive:
                {
                output._DoOutputLine("OpCode::CurvePrimitive\n");
                break;
                }

            case GeometryStreamIO::OpCode::SolidPrimitive:
                {
                output._DoOutputLine("OpCode::SolidPrimitive\n");
                break;
                }

            case GeometryStreamIO::OpCode::BsplineSurface:
                {
                output._DoOutputLine("OpCode::BsplineSurface\n");
                break;
                }

            case GeometryStreamIO::OpCode::ParasolidBRep:
                {
                output._DoOutputLine("OpCode::ParasolidBRep\n");
                break;
                }

            case GeometryStreamIO::OpCode::BRepPolyfaceDeprecated:
                {
                output._DoOutputLine("OpCode::BRepPolyfaceDeprecated\n");
                break;
                }

            case GeometryStreamIO::OpCode::BRepCurveVecDeprecated:
                {
                output._DoOutputLine("OpCode::BRepCurveVecDeprecated\n");
                break;
                }

            case GeometryStreamIO::OpCode::AreaFill:
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

                    output._DoOutputLine("\n");
                    break;
                    }

                if (!(ppfb->has_color() || ppfb->has_useColor() || ppfb->has_backgroundFill() || ppfb->has_transparency()))
                    break;

                output._DoOutputLine("  ");

                if (ppfb->has_color() || ppfb->has_useColor())
                    {
                    ColorDef color(ppfb->color());
                    output._DoOutputLine(Utf8PrintfString("Fill: [Red:%d Green:%d Blue:%d Alpha:%d] ", color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha()).c_str());
                    }
                else if (ppfb->has_backgroundFill())
                    {
                    output._DoOutputLine(Utf8PrintfString("Fill: View Background %s", 1 == ppfb->backgroundFill() ? "Solid" : "Outline").c_str());
                    }
                else
                    {
                    output._DoOutputLine("Fill: Single Color ");
                    }

                if (ppfb->has_transparency())
                    output._DoOutputLine(Utf8PrintfString("Transparency: %lf ", ppfb->transparency()).c_str());

                output._DoOutputLine("\n");
                break;
                }

            case GeometryStreamIO::OpCode::Pattern:
                {
                output._DoOutputLine("OpCode::Pattern\n");
                break;
                }

            case GeometryStreamIO::OpCode::Material:
                {
                output._DoOutputLine("OpCode::Material\n");
                break;
                }

            case GeometryStreamIO::OpCode::LineStyleModifiers:
                {
                output._DoOutputLine("OpCode::LineStyleModifiers\n");
                break;
                }

            case GeometryStreamIO::OpCode::TextString:
                {
                output._DoOutputLine("OpCode::TextString\n");
                break;
                }

            case GeometryStreamIO::OpCode::Image:
                {
                output._DoOutputLine("OpCode::Image\n");
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
        for (DgnGeometryPartId partId : parts)
            {
            DgnGeometryPartCPtr partGeometry = db.Elements().Get<DgnGeometryPart>(partId);

            if (!partGeometry.IsValid())
                continue;

            output._DoOutputLine(Utf8PrintfString("\n[--- Part: %s - Id: %" PRIu64 " ---]\n\n", partGeometry->GetCode().GetValueUtf8CP(), partId.GetValue()).c_str());
            GeometryStreamIO::Debug(output, partGeometry->GetGeometryStream(), db, true);
            }
        }

    if (output._WantGeomEntryIds() && !isPart)
        {
        GeometryCollection geomCollection(stream, db);

        output._DoOutputLine(Utf8PrintfString("\n--- GeometryStream Entry Ids ---\n\n").c_str());

        for (auto iter : geomCollection)
            {
            GeometricPrimitivePtr geom = iter.GetGeometryPtr();

            if (geom.IsValid())
                {
                debugGeomId(output, *geom, iter.GetGeometryStreamEntryId());
                continue;
                }

            DgnGeometryPartCPtr partGeom = iter.GetGeometryPartCPtr();

            if (!partGeom.IsValid())
                continue;

            GeometryCollection partCollection(partGeom->GetGeometryStream(), db);

            partCollection.SetNestedIteratorContext(iter); // Iterate part GeomStream in context of parent...

            for (auto partIter : partCollection)
                {
                GeometricPrimitivePtr geomInPart = partIter.GetGeometryPtr();

                if (!geomInPart.IsValid())
                    continue;

                debugGeomId(output, *geomInPart, partIter.GetGeometryStreamEntryId());
                }
            }

        output._DoOutputLine("\n");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Collection::GetGeometryPartIds(IdSet<DgnGeometryPartId>& parts, DgnDbR dgnDb) const
    {
    GeometryStreamIO::Reader reader(dgnDb);

    for (auto const& egOp : *this)
        {
        if (GeometryStreamIO::OpCode::GeometryPartInstance != egOp.m_opCode)
            continue;

        DgnGeometryPartId geomPartId;
        Transform geomToElem;

        if (!reader.Get(egOp, geomPartId, geomToElem))
            continue;

        parts.insert(geomPartId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryStreamIO::Header GeometryStreamIO::Collection::GetHeader() const
    {
    // 00 Opcode::Header
    // 04 Data length
    // 08 Header
    constexpr size_t headerOffset = sizeof(uint32_t) * 2;
    Header header;
    if (headerOffset + sizeof(Header) <= m_dataSize)
        header = *reinterpret_cast<Header const*>(m_data + headerOffset);

    return header;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DrawHelper
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void CookGeometryParams(ViewContextR context, Render::GeometryParamsR geomParams, Render::GraphicBuilderR graphic, bool& geomParamsChanged)
    {
    if (!geomParamsChanged)
        {
        // NOTE: Even if we aren't activating, make sure to resolve so that we can test for linestyles, display priority, etc.
        geomParams.Resolve(context);
        return;
        }

    context.CookGeometryParams(geomParams, graphic);
    geomParamsChanged = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsGeometryVisible(ViewContextR context, Render::GeometryParamsCR geomParams, DRange3dCP range)
    {
    return context.IsGeometryVisible(geomParams, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsFillVisible(ViewContextR context, Render::GeometryParamsCR geomParams)
    {
    // NEEDSWORK Mesh tiles...will want to control fill with shader so that turning it on/off doesn't invalidate tiles...
    switch (geomParams.GetFillDisplay())
        {
        case FillDisplay::Never:
            return false;

        case FillDisplay::ByView:
            return context.GetViewFlags().ShowFill();

        default:
            return true;
        }
    }

}; // DrawHelper

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Collection::Draw(Render::GraphicBuilderR mainGraphic, ViewContextR context, Render::GeometryParamsR geomParams, bool activateParams, DgnElementCP element) const
    {
    bool geomParamsChanged = true;
    bool allowStrokedLinestyles = true; // Allow stroked linestyles in GeometryParts...just don't instance...
    Render::GraphicParams subGraphicParams;
    DRange3d subGraphicRange = DRange3d::NullRange();
    Render::GraphicBuilderPtr subGraphic;
    Render::GraphicBuilderP currGraphic = &mainGraphic;
    GeometryStreamEntryIdCP currEntryId = currGraphic->GetGeometryStreamEntryId();
    GeometryStreamEntryId entryId;
    GeometryStreamIO::Reader reader(context.GetDgnDb());

    if (nullptr != currEntryId)
        entryId = *currEntryId;

    for (auto const& egOp : *this)
        {
        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                {
                entryId.SetActive(true);
                currGraphic->SetGeometryStreamEntryId(&entryId);
                break;
                }

            case GeometryStreamIO::OpCode::BasicSymbology:
            case GeometryStreamIO::OpCode::LineStyleModifiers:
            case GeometryStreamIO::OpCode::AreaFill:
            case GeometryStreamIO::OpCode::Pattern:
            case GeometryStreamIO::OpCode::Material:
                {
                if (!reader.Get(egOp, geomParams))
                    break;

                geomParamsChanged = true;
                break;
                }

            case GeometryStreamIO::OpCode::SubGraphicRange:
                {
                currGraphic = &mainGraphic;
                subGraphicRange = DRange3d::NullRange();

                if (!reader.Get(egOp, subGraphicRange))
                    break;

                subGraphic = mainGraphic.CreateSubGraphic(Transform::FromIdentity());
                currGraphic = subGraphic.get();

                mainGraphic.GetLocalToWorldTransform().Multiply(subGraphicRange, subGraphicRange); // Range test needs world coords...

                geomParams.Resolve(context);
                subGraphicParams.Cook(geomParams, context); // Save current params for AddSubGraphic in case there are additional symbology changes...

                currGraphic->ActivateGraphicParams(subGraphicParams, &geomParams);
                geomParamsChanged = false;

                continue; // Next op code should be a geometry op code that will be added to this sub-graphic...
                }

            case GeometryStreamIO::OpCode::GeometryPartInstance:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                DgnGeometryPartId geomPartId;
                Transform geomToSource;

                if (!reader.Get(egOp, geomPartId, geomToSource))
                    break;

                entryId.SetActiveGeometryPart(geomPartId);
                currGraphic->SetGeometryStreamEntryId(&entryId);

                context.AddSubGraphic(mainGraphic, geomPartId, geomToSource, geomParams);

                entryId.SetActiveGeometryPart(DgnGeometryPartId());
                currGraphic->SetGeometryStreamEntryId(&entryId);

                break;
                }

            case GeometryStreamIO::OpCode::PointPrimitive2d:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                int         nPts;
                int8_t      boundary;
                DPoint2dCP  pts;

                if (!reader.Get(egOp, pts, nPts, boundary))
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);

                if (FB::BoundaryType_Closed == boundary)
                    {
                    PatternParamsCP pattern;

                    if (nullptr != (pattern = geomParams.GetPatternParams()))
                        {
                        if (context.WantAreaPatterns())
                            {
                            std::valarray<DPoint3d> localPoints3dBuf(nPts);

                            for (int iPt = 0; iPt < nPts; ++iPt)
                                localPoints3dBuf[iPt].Init(pts[iPt]);

                            context.DrawAreaPattern(*currGraphic, *CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(&localPoints3dBuf[0], nPts)), geomParams, false);
                            }

                        if (pattern->GetInvisibleBoundary() && !DrawHelper::IsFillVisible(context, geomParams))
                            break;
                        }
                    }

                if (FB::BoundaryType_None != boundary)
                    {
                    bool strokeLineStyle = (allowStrokedLinestyles && context.WantLineStyles() && geomParams.HasStrokedLineStyle());

                    if (strokeLineStyle)
                        {
                        std::valarray<DPoint3d> localPoints3dBuf(nPts);

                        for (int iPt = 0; iPt < nPts; ++iPt)
                            localPoints3dBuf[iPt].Init(pts[iPt]);

                        context.DrawStyledCurveVector(*currGraphic, *CurveVector::Create(FB::BoundaryType_Closed == boundary ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLineString(&localPoints3dBuf[0], nPts)), geomParams, false);
                        break;
                        }
                    }

                switch (boundary)
                    {
                    case FB::BoundaryType_None:
                        currGraphic->AddPointString2d(nPts, pts, geomParams.GetNetDisplayPriority());
                        break;

                    case FB::BoundaryType_Open:
                        currGraphic->AddLineString2d(nPts, pts, geomParams.GetNetDisplayPriority());
                        break;

                    case FB::BoundaryType_Closed:
                        currGraphic->AddShape2d(nPts, pts, DrawHelper::IsFillVisible(context, geomParams), geomParams.GetNetDisplayPriority());
                        break;
                    }
                break;
                }

            case GeometryStreamIO::OpCode::PointPrimitive:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                int         nPts;
                int8_t      boundary;
                DPoint3dCP  pts;

                if (!reader.Get(egOp, pts, nPts, boundary))
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);

                if (FB::BoundaryType_Closed == boundary)
                    {
                    PatternParamsCP pattern;

                    if (nullptr != (pattern = geomParams.GetPatternParams()))
                        {
                        if (context.WantAreaPatterns())
                            context.DrawAreaPattern(*currGraphic, *CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(pts, nPts)), geomParams, false);

                        if (pattern->GetInvisibleBoundary() && !DrawHelper::IsFillVisible(context, geomParams))
                            break;
                        }
                    }

                if (FB::BoundaryType_None != boundary)
                    {
                    bool strokeLineStyle = (allowStrokedLinestyles && context.WantLineStyles() && geomParams.HasStrokedLineStyle());

                    if (strokeLineStyle)
                        {
                        context.DrawStyledCurveVector(*currGraphic, *CurveVector::Create(FB::BoundaryType_Closed == boundary ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLineString(pts, nPts)), geomParams, false);
                        break;
                        }
                    }

                switch (boundary)
                    {
                    case FB::BoundaryType_None:
                        currGraphic->AddPointString(nPts, pts);
                        break;

                    case FB::BoundaryType_Open:
                        currGraphic->AddLineString(nPts, pts);
                        break;

                    case FB::BoundaryType_Closed:
                        currGraphic->AddShape(nPts, pts, DrawHelper::IsFillVisible(context, geomParams));
                        break;
                    }
                break;
                }

            case GeometryStreamIO::OpCode::ArcPrimitive:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                DEllipse3d  arc;
                int8_t      boundary;

                if (!reader.Get(egOp, arc, boundary))
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);

                if (FB::BoundaryType_Closed == boundary)
                    {
                    PatternParamsCP pattern;

                    if (nullptr != (pattern = geomParams.GetPatternParams()))
                        {
                        if (context.WantAreaPatterns())
                            context.DrawAreaPattern(*currGraphic, *CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateArc(arc)), geomParams, false);

                        if (pattern->GetInvisibleBoundary() && !DrawHelper::IsFillVisible(context, geomParams))
                            break;
                        }
                    }

                bool strokeLineStyle = (allowStrokedLinestyles && context.WantLineStyles() && geomParams.HasStrokedLineStyle());

                if (strokeLineStyle)
                    {
                    context.DrawStyledCurveVector(*currGraphic, *CurveVector::Create(FB::BoundaryType_Closed == boundary ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(arc)), geomParams, false);
                    break;
                    }

                if (!context.Is3dView())
                    {
                    if (FB::BoundaryType_Closed != boundary)
                        currGraphic->AddArc2d(arc, false, false, geomParams.GetNetDisplayPriority());
                    else
                        currGraphic->AddArc2d(arc, true, DrawHelper::IsFillVisible(context, geomParams), geomParams.GetNetDisplayPriority());
                    break;
                    }

                if (FB::BoundaryType_Closed != boundary)
                    currGraphic->AddArc(arc, false, false);
                else
                    currGraphic->AddArc(arc, true, DrawHelper::IsFillVisible(context, geomParams));
                break;
                }

            case GeometryStreamIO::OpCode::CurvePrimitive:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                ICurvePrimitivePtr curvePrimitivePtr;

                if (!reader.Get(egOp, curvePrimitivePtr))
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);

                if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == curvePrimitivePtr->GetCurvePrimitiveType())
                    {
                    if (!context.Is3dView())
                        {
                        bvector<DPoint3d> const* points = curvePrimitivePtr->GetPointStringCP();
                        int nPts = (int) points->size();
                        std::valarray<DPoint2d> localPoints2dBuf(nPts);

                        for (int iPt = 0; iPt < nPts; ++iPt)
                            {
                            DPoint3dCP tmpPt = &points->front()+iPt;

                            localPoints2dBuf[iPt].x = tmpPt->x;
                            localPoints2dBuf[iPt].y = tmpPt->y;
                            }

                        currGraphic->AddPointString2d(nPts, &localPoints2dBuf[0], geomParams.GetNetDisplayPriority());
                        break;
                        }

                    currGraphic->AddPointString((int) curvePrimitivePtr->GetPointStringCP()->size(), &curvePrimitivePtr->GetPointStringCP()->front());
                    break;
                    }

                CurveVectorPtr curvePtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curvePrimitivePtr); // A single curve primitive (that isn't a point string) is always open...
                bool strokeLineStyle = (allowStrokedLinestyles && context.WantLineStyles() && geomParams.HasStrokedLineStyle());

                if (strokeLineStyle)
                    {
                    context.DrawStyledCurveVector(*currGraphic, *curvePtr, geomParams, false);
                    break;
                    }

                if (!context.Is3dView())
                    {
                    currGraphic->AddCurveVector2dR(*curvePtr, false, geomParams.GetNetDisplayPriority());
                    break;
                    }

                currGraphic->AddCurveVectorR(*curvePtr, false);
                break;
                }

            case GeometryStreamIO::OpCode::CurveVector:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                CurveVectorPtr curvePtr;

                if (!reader.Get(egOp, curvePtr))
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);

                if (curvePtr->IsAnyRegionType())
                    {
                    PatternParamsCP pattern;

                    if (nullptr != (pattern = geomParams.GetPatternParams()))
                        {
                        if (context.WantAreaPatterns())
                            context.DrawAreaPattern(*currGraphic, *curvePtr, geomParams, false);

                        if (pattern->GetInvisibleBoundary() && !DrawHelper::IsFillVisible(context, geomParams))
                            break;
                        }
                    }

                bool strokeLineStyle = (allowStrokedLinestyles && context.WantLineStyles() && geomParams.HasStrokedLineStyle());

                if (strokeLineStyle)
                    {
                    context.DrawStyledCurveVector(*currGraphic, *curvePtr, geomParams, false);
                    break;
                    }

                if (!context.Is3dView())
                    {
                    currGraphic->AddCurveVector2dR(*curvePtr, curvePtr->IsAnyRegionType() && DrawHelper::IsFillVisible(context, geomParams), geomParams.GetNetDisplayPriority());
                    break;
                    }

                currGraphic->AddCurveVectorR(*curvePtr, curvePtr->IsAnyRegionType() && DrawHelper::IsFillVisible(context, geomParams));

                // NOTE: We no longer want to support the surface/control polygon visibility options.
                //       Display of the control polygon is something that should be left to specific tools and modify handles.
                //       WireframeGeomUtil::DrawControlPolygon was created to help tools that wish to show the control polygon.
                break;
                }

            case GeometryStreamIO::OpCode::Polyface:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                PolyfaceQueryCarrier meshData(0, false, 0, 0, nullptr, nullptr);

                if (!reader.Get(egOp, meshData))
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);
                currGraphic->AddPolyface(meshData, DrawHelper::IsFillVisible(context, geomParams));
                break;
                };

            case GeometryStreamIO::OpCode::SolidPrimitive:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                ISolidPrimitivePtr solidPtr;

                if (!reader.Get(egOp, solidPtr))
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);
                currGraphic->AddSolidPrimitiveR(*solidPtr);
                break;
                }

            case GeometryStreamIO::OpCode::BsplineSurface:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                MSBsplineSurfacePtr surfacePtr;

                if (!reader.Get(egOp, surfacePtr))
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);
                currGraphic->AddBSplineSurfaceR(*surfacePtr);

                // NOTE: We no longer want to support the surface/control polygon visibility options.
                //       Display of the control polygon is something that should be left to specific tools and modify handles.
                //       WireframeGeomUtil::DrawControlPolygon was created to help tools that wish to show the control polygon.
                break;
                }

            case GeometryStreamIO::OpCode::ParasolidBRep:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                // Try to get a BRepHandle to avoid instantiating the BRep.
                auto readBRepData = [&]()
                    {
                    IBRepDataSupplier::Data data = { egOp.m_data, egOp.m_dataSize };
                    return data;
                    };

                BRepDataSupplier<decltype(readBRepData)> supplier(readBRepData);
                auto brepHandle = currGraphic->GetBRepHandle(entryId, supplier);
                if (brepHandle.IsValid())
                    {
                    DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);
                    currGraphic->AddBRepHandle(*brepHandle);
                    break;
                    }

                // Deserialize and add the BRep.
                IBRepEntityPtr entityPtr;
                if (!reader.Get(egOp, entityPtr) || !entityPtr.IsValid())
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);
                currGraphic->AddBodyR(*entityPtr);
                break;
                }

            case GeometryStreamIO::OpCode::TextString:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                TextString  text(context.GetDgnDb());
                if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(text, egOp.m_data, egOp.m_dataSize))
                    break;

                if (text.GetGlyphSymbology(geomParams))
                    geomParamsChanged = true;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);

                if (!context.Is3dView())
                    {
                    currGraphic->AddTextString2d(text, geomParams.GetNetDisplayPriority());
                    break;
                    }

                currGraphic->AddTextString(text);
                break;
                }

            case GeometryStreamIO::OpCode::Image:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                ImageGraphicPtr imagePtr;
                if (!reader.Get(egOp, imagePtr))
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);

                if (context.Is3dView())
                    currGraphic->AddImageR(*imagePtr);
                else
                    currGraphic->AddImage2dR(*imagePtr, geomParams.GetNetDisplayPriority());

                break;
                }

            default:
                break;
            }

        if (subGraphic.IsValid())
            {
            mainGraphic.AddSubGraphic(*subGraphic->Finish(), Transform::FromIdentity(), subGraphicParams);

            currGraphic = &mainGraphic;
            subGraphic = nullptr;
            }

        if (context.CheckStop())
            break;
        }

    entryId.SetActive(false);
    currGraphic->SetGeometryStreamEntryId(&entryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr GeometrySource::_Stroke(ViewContextR context, double pixelSize) const
    {
    return Draw(context, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr GeometrySource::Draw(ViewContextR context, double pixelSize) const
    {
    auto graphic = context.CreateSceneGraphic(GetPlacementTransform());
    Render::GeometryParams params;

    params.SetCategoryId(GetCategoryId());
    GeometryStreamIO::Collection(GetGeometryStream().GetData(), GetGeometryStream().GetSize()).Draw(*graphic, context, params, true, ToElement());

    return graphic->Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryCollection::Iterator::EntryType GeometryCollection::Iterator::GetEntryType() const
    {
    switch (m_egOp.m_opCode)
        {
        case GeometryStreamIO::OpCode::GeometryPartInstance:
            return EntryType::GeometryPart;

        case GeometryStreamIO::OpCode::PointPrimitive:
            {
            auto ppfb = flatbuffers::GetRoot<FB::PointPrimitive>(m_egOp.m_data);

            return (FB::BoundaryType_Closed == ppfb->boundary() ? EntryType::CurveVector : EntryType::CurvePrimitive);
            }

        case GeometryStreamIO::OpCode::PointPrimitive2d:
            {
            auto ppfb = flatbuffers::GetRoot<FB::PointPrimitive2d>(m_egOp.m_data);

            return (FB::BoundaryType_Closed == ppfb->boundary() ? EntryType::CurveVector : EntryType::CurvePrimitive);
            }

        case GeometryStreamIO::OpCode::ArcPrimitive:
            {
            auto ppfb = flatbuffers::GetRoot<FB::ArcPrimitive>(m_egOp.m_data);

            return (FB::BoundaryType_Closed == ppfb->boundary() ? EntryType::CurveVector : EntryType::CurvePrimitive);
            }

        case GeometryStreamIO::OpCode::CurvePrimitive:
            return EntryType::CurvePrimitive;

        case GeometryStreamIO::OpCode::CurveVector:
            return EntryType::CurveVector;

        case GeometryStreamIO::OpCode::Polyface:
            return EntryType::Polyface;

        case GeometryStreamIO::OpCode::SolidPrimitive:
            return EntryType::SolidPrimitive;

        case GeometryStreamIO::OpCode::BsplineSurface:
            return EntryType::BsplineSurface;

        case GeometryStreamIO::OpCode::ParasolidBRep:
            return EntryType::BRepEntity;

        case GeometryStreamIO::OpCode::TextString:
            return EntryType::TextString;

        case GeometryStreamIO::OpCode::Image:
            return EntryType::Image;

        default:
            BeAssert(false); return EntryType::Unknown;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryCollection::Iterator::IsCurve() const
    {
    switch (m_egOp.m_opCode)
        {
        case GeometryStreamIO::OpCode::PointPrimitive:
            {
            auto ppfb = flatbuffers::GetRoot<FB::PointPrimitive>(m_egOp.m_data);

            return (FB::BoundaryType_Open == ppfb->boundary());
            }

        case GeometryStreamIO::OpCode::PointPrimitive2d:
            {
            auto ppfb = flatbuffers::GetRoot<FB::PointPrimitive2d>(m_egOp.m_data);

            return (FB::BoundaryType_Open == ppfb->boundary());
            }

        case GeometryStreamIO::OpCode::ArcPrimitive:
            {
            auto ppfb = flatbuffers::GetRoot<FB::ArcPrimitive>(m_egOp.m_data);

            return (FB::BoundaryType_Open == ppfb->boundary());
            }

        case GeometryStreamIO::OpCode::CurvePrimitive:
            {
            return true; // NOTE: Should never be a point string or closed bcurve...
            }

        case GeometryStreamIO::OpCode::CurveVector:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && !geom->IsSheet()); // Accept "none" boundary type...
            }

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryCollection::Iterator::IsSurface() const
    {
    switch (m_egOp.m_opCode)
        {
        case GeometryStreamIO::OpCode::PointPrimitive:
            {
            auto ppfb = flatbuffers::GetRoot<FB::PointPrimitive>(m_egOp.m_data);

            return (FB::BoundaryType_Closed == ppfb->boundary());
            }

        case GeometryStreamIO::OpCode::PointPrimitive2d:
            {
            auto ppfb = flatbuffers::GetRoot<FB::PointPrimitive2d>(m_egOp.m_data);

            return (FB::BoundaryType_Closed == ppfb->boundary());
            }

        case GeometryStreamIO::OpCode::ArcPrimitive:
            {
            auto ppfb = flatbuffers::GetRoot<FB::ArcPrimitive>(m_egOp.m_data);

            return (FB::BoundaryType_Closed == ppfb->boundary());
            }

        case GeometryStreamIO::OpCode::CurvePrimitive:
            {
            return false; // NOTE: Should never be a point string or closed bcurve...
            }

        case GeometryStreamIO::OpCode::CurveVector:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && geom->IsSheet());
            }

        case GeometryStreamIO::OpCode::SolidPrimitive:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && geom->IsSheet());
            }

        case GeometryStreamIO::OpCode::Polyface:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && geom->IsSheet());
            }

        case GeometryStreamIO::OpCode::BsplineSurface:
            {
            return true;
            }

        case GeometryStreamIO::OpCode::ParasolidBRep:
            {
            auto ppfb = flatbuffers::GetRoot<FB::BRepData>(m_egOp.m_data);

            return (IBRepEntity::EntityType::Sheet == ((IBRepEntity::EntityType) ppfb->brepType()));
            }

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryCollection::Iterator::IsSolid() const
    {
    switch (m_egOp.m_opCode)
        {
        case GeometryStreamIO::OpCode::SolidPrimitive:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && geom->IsSolid());
            }

        case GeometryStreamIO::OpCode::Polyface:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && geom->IsSolid());
            }

        case GeometryStreamIO::OpCode::ParasolidBRep:
            {
            auto ppfb = flatbuffers::GetRoot<FB::BRepData>(m_egOp.m_data);

            return (IBRepEntity::EntityType::Solid == ((IBRepEntity::EntityType) ppfb->brepType()));
            }

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GeometryCollection::Iterator::GetGeometryPtr() const
    {
    if (m_state->m_geometry.IsValid())
        return m_state->m_geometry;

    GeometryStreamIO::Reader reader(m_state->m_dgnDb);

    if (!reader.Get(m_egOp, m_state->m_geometry))
        return nullptr;

    return m_state->m_geometry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryCollection::Iterator::ToNext()
    {
    do
        {
        if (m_dataOffset >= m_dataSize)
            {
            m_state->m_geomStreamEntryId.SetActive(false);
            m_data = nullptr;
            m_dataOffset = 0;
            return;
            }

        // NOTE: Don't want to clear partId and transform when using nested iter for GeometryPart...
        if (0 != m_dataOffset && GeometryStreamIO::OpCode::GeometryPartInstance == m_egOp.m_opCode)
            {
            m_state->m_geomStreamEntryId.SetActive(false);
            m_state->m_geomToSource = Transform::FromIdentity();
            }

        uint32_t        opCode = *((uint32_t const*) (m_data));
        uint32_t        dataSize = *((uint32_t const*) (m_data + sizeof (opCode)));
        uint8_t const*  data = (0 != dataSize ? (uint8_t const*) (m_data + sizeof (opCode) + sizeof (dataSize)) : nullptr);
        size_t          egOpSize = sizeof (opCode) + sizeof (dataSize) + dataSize;

        m_egOp = GeometryStreamIO::Operation((GeometryStreamIO::OpCode) (opCode), dataSize, data);
        m_data += egOpSize;
        m_dataOffset += egOpSize;

        GeometryStreamIO::Reader reader(m_state->m_dgnDb);

        switch (m_egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                {
                m_state->m_geomStreamEntryId.SetActive(true);
                break;
                }

            case GeometryStreamIO::OpCode::BasicSymbology:
            case GeometryStreamIO::OpCode::LineStyleModifiers:
            case GeometryStreamIO::OpCode::AreaFill:
            case GeometryStreamIO::OpCode::Pattern:
            case GeometryStreamIO::OpCode::Material:
                {
                reader.Get(m_egOp, m_state->m_geomParams); // Update active GeometryParams...
                break;
                }

            case GeometryStreamIO::OpCode::SubGraphicRange:
                {
                reader.Get(m_egOp, m_state->m_localRange);
                break;
                }

            case GeometryStreamIO::OpCode::GeometryPartInstance:
                {
                DgnGeometryPartId geomPartId;
                Transform         geomToSource;

                m_state->m_geomStreamEntryId.Increment();

                if (!reader.Get(m_egOp, geomPartId, geomToSource))
                    break;

                m_state->m_geomStreamEntryId.SetActiveGeometryPart(geomPartId);
                m_state->m_geomToSource = geomToSource;
                m_state->m_localRange = DRange3d::NullRange();
                m_state->m_geometry = nullptr;

                if (m_state->m_geomParams.GetCategoryId().IsValid())
                    m_state->m_geomParams.Resolve(m_state->m_dgnDb); // Resolve sub-category appearance...
                return;
                }

            default:
                {
                if (!m_egOp.IsGeometryOp())
                    break;

                m_state->m_geomStreamEntryId.Increment();
                m_state->m_geometry = nullptr; // Defer extract until asked...

                if (m_state->m_geomParams.GetCategoryId().IsValid())
                    m_state->m_geomParams.Resolve(m_state->m_dgnDb); // Resolve sub-category appearance...
                return;
                }
            }

        } while (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryStreamIO::Header::Flags GeometryCollection::GetHeaderFlags() const
    {
    GeometryStreamIO::Collection collection(m_data, m_dataSize);
    return collection.GetHeader().m_flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryCollection::SetNestedIteratorContext(Iterator const& iter)
    {
    m_state.m_geomParams = iter.m_state->m_geomParams;
    m_state.m_geomStreamEntryId = iter.m_state->m_geomStreamEntryId;
    m_state.m_sourceToWorld = iter.m_state->m_sourceToWorld;
    m_state.m_geomToSource = iter.m_state->m_geomToSource;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryCollection::GeometryCollection(GeometryStreamCR geom, DgnDbR dgnDb, Render::GeometryParamsCP baseParams, TransformCP sourceToWorld) : m_state(dgnDb)
    {
    m_data = geom.GetData();
    m_dataSize = geom.GetSize();

    if (nullptr != baseParams)
        m_state.m_geomParams = *baseParams;

    if (nullptr != sourceToWorld)
        m_state.m_sourceToWorld = *sourceToWorld;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryCollection::GeometryCollection(GeometrySourceCR source) : m_state(source.GetSourceDgnDb())
    {
    m_data = source.GetGeometryStream().GetData();
    m_dataSize = source.GetGeometryStream().GetSize();
    m_state.m_geomParams.SetCategoryId(source.GetCategoryId());
    m_state.m_sourceToWorld = source.GetPlacementTransform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryCollection::ToJson(BeJsValue output, BeJsConst opts) const
    {
    output.SetNull();
    GeometryStreamIO::Collection collection(m_data, m_dataSize);
    GeometryStreamIO::Reader reader(m_state.m_dgnDb);
    DgnSubCategoryId lastSubCategory = m_state.m_geomParams.GetSubCategoryId();
    bool wantBRepData = opts["wantBRepData"].asBool();

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                {
                auto header = output.appendValue()["header"];
                header["flags"] = static_cast<uint32_t>(reader.GetHeader(egOp)->m_flags);
                break;
                }

            case GeometryStreamIO::OpCode::BasicSymbology:
                {
                auto ppfb = flatbuffers::GetRoot<FB::BasicSymbology>(egOp.m_data);
                auto value = output.appendObject()["appearance"]; // NOTE: A null "appearance" indicates a reset to default sub-category appearance...
                value.SetEmptyObject();
                DgnSubCategoryId subCategoryId((uint64_t)ppfb->subCategoryId());

                // NOTE: Don't include default sub-category if sub-category isn't changing from a non-default sub-category (was un-necessarily included in some GeometryStreams)...
                if (subCategoryId.IsValid() && subCategoryId != lastSubCategory)
                    {
                    value["subCategory"] = subCategoryId.ToHexStr();
                    lastSubCategory = subCategoryId;
                    }

                if (ppfb->useColor())
                    value["color"] = ppfb->color();

                if (ppfb->useWeight())
                    value["weight"] = ppfb->weight();

                if (ppfb->useStyle())
                    {
                    DgnStyleId styleId((uint64_t)ppfb->lineStyleId());
                    value["style"] = styleId.ToHexStr();
                    }

                if (0.0 != ppfb->transparency())
                    value["transparency"] = ppfb->transparency();

                if (0 != ppfb->displayPriority())
                    value["displayPriority"] = ppfb->displayPriority();

                if (FB::GeometryClass_Primary != ppfb->geomClass())
                    value["geometryClass"] = ppfb->geomClass();

                break;
                }

            case GeometryStreamIO::OpCode::LineStyleModifiers:
                {
                auto value = output.appendObject()["styleMod"];
                auto ppfb = flatbuffers::GetRoot<FB::LineStyleModifiers>(egOp.m_data);
                uint32_t modifiers = ppfb->modifiers();

                if (0 != (STYLEMOD_SCALE & modifiers) && 0.0 != ppfb->scale())
                    value["scale"] = ppfb->scale();

                if (0 != (STYLEMOD_DSCALE & modifiers) && 0.0 != ppfb->dashScale())
                    value["dashScale"] = ppfb->dashScale();

                if (0 != (STYLEMOD_GSCALE & modifiers) && 0.0 != ppfb->gapScale())
                    value["gapScale"] = ppfb->gapScale();

                if (0 != (STYLEMOD_SWIDTH & modifiers))
                    value["startWidth"] = ppfb->startWidth();

                if (0 != (STYLEMOD_EWIDTH & modifiers))
                    value["endWidth"] = ppfb->endWidth();

                if (0 != (STYLEMOD_DISTPHASE & modifiers))
                    value["distPhase"] = ppfb->distPhase();

                if (0 != (STYLEMOD_FRACTPHASE & modifiers))
                    value["fractPhase"] = ppfb->fractPhase();

                if (0 != (STYLEMOD_CENTERPHASE & modifiers))
                    value["centerPhase"] = true;

                if (0 != (STYLEMOD_NOSEGMODE & modifiers))
                    value["segmentMode"] = false;
                else if (0 != (STYLEMOD_SEGMODE & modifiers))
                    value["segmentMode"] = true;

                if (0 != (STYLEMOD_TRUE_WIDTH & modifiers))
                    value["physicalWidth"] = true;

                if (0 != (STYLEMOD_NORMAL & modifiers))
                    BeJsGeomUtils::DPoint3dToJson(value["normal"], *(DPoint3dCP) ppfb->normal());

                if (0 != (STYLEMOD_RMATRIX & modifiers))
                    BeJsGeomUtils::YawPitchRollToJson(value["rotation"], YawPitchRollAngles(AngleInDegrees::FromDegrees(ppfb->yaw()), AngleInDegrees::FromDegrees(ppfb->pitch()), AngleInDegrees::FromDegrees(ppfb->roll())));

                break;
                }

            case GeometryStreamIO::OpCode::AreaFill:
                {
                auto ppfb = flatbuffers::GetRoot<FB::AreaFill>(egOp.m_data);
                auto value = output.appendValue()["fill"];

                value["display"] = ppfb->fill();

                if (0 != ppfb->fill())
                    {
                    if (0.0 != ppfb->transparency())
                        value["transparency"] = ppfb->transparency();

                    if (0 == ppfb->mode())
                        {
                        if (ppfb->useColor())
                            value["color"] = ppfb->color();
                        else if (0 != ppfb->backgroundFill())
                            value["backgroundFill"] = ppfb->backgroundFill();
                        }
                    else
                        {
                        GradientSymbPtr gradientPtr = GradientSymb::Create();

                        gradientPtr->SetMode((GradientSymb::Mode)ppfb->mode());
                        gradientPtr->SetFlags((GradientSymb::Flags)ppfb->flags());
                        gradientPtr->SetShift(ppfb->shift());
                        gradientPtr->SetTint(ppfb->tint());
                        gradientPtr->SetAngle(ppfb->angle());

                        uint32_t nColors = ppfb->colors()->Length();
                        uint32_t const* colors = (uint32_t const*) ppfb->colors()->Data();
                        bvector<ColorDef> keyColors;

                        for (uint32_t iColor=0; iColor < nColors; ++iColor)
                            keyColors.push_back(ColorDef(colors[iColor]));

                        gradientPtr->SetKeys((uint32_t) keyColors.size(), &keyColors.front(), (double const*) ppfb->values()->Data());

                        if (GradientSymb::Mode::Thematic == gradientPtr->GetMode() && 0 != ppfb->thematicSettings())
                            {
                            auto    thematicSettings = new ThematicGradientSettings();
                            auto    fbThematicSettings = ppfb->thematicSettings();

                            thematicSettings->SetStepCount(fbThematicSettings->stepCount());
                            thematicSettings->SetMarginColor(ColorDef(fbThematicSettings->marginColor()));
                            thematicSettings->SetMode((ThematicGradientSettings::Mode) fbThematicSettings->mode());
                            thematicSettings->SetColorScheme((ThematicGradientSettings::ColorScheme) fbThematicSettings->colorScheme());
                            thematicSettings->SetRange(*((DRange1dCP) fbThematicSettings->range()));
                            gradientPtr->SetThematicSettings(*thematicSettings);
                            }

                         gradientPtr->ToJson(value["gradient"]);
                        }
                    }

                break;
                }

            case GeometryStreamIO::OpCode::Pattern:
                {
                auto ppfb = flatbuffers::GetRoot<FB::AreaPattern>(egOp.m_data);
                PatternParamsPtr pattern = PatternParams::Create();

                if (ppfb->has_origin())
                    pattern->SetOrigin(*((DPoint3dCP) ppfb->origin()));

                if (ppfb->has_rotation())
                    pattern->SetOrientation(*((RotMatrixCP) ppfb->rotation()));

                pattern->SetPrimarySpacing(ppfb->space1());
                pattern->SetSecondarySpacing(ppfb->space2());
                pattern->SetPrimaryAngle(ppfb->angle1());
                pattern->SetSecondaryAngle(ppfb->angle2());
                pattern->SetScale(ppfb->scale());

                if (ppfb->useColor())
                    pattern->SetColor(ColorDef(ppfb->color()));

                if (ppfb->useWeight())
                    pattern->SetWeight(ppfb->weight());

                pattern->SetInvisibleBoundary(TO_BOOL(ppfb->invisibleBoundary()));
                pattern->SetSnappable(TO_BOOL(ppfb->snappable()));
                pattern->SetSymbolId(DgnGeometryPartId((uint64_t) ppfb->symbolId()));

                if (ppfb->has_defLine())
                    {
                    flatbuffers::Vector<flatbuffers::Offset<FB::DwgHatchDefLine>> const* fbDefLineOffsets = ppfb->defLine();
                    bvector<DwgHatchDefLine> defLines;

                    for (auto const& fbDefLine : *fbDefLineOffsets)
                        {
                        DwgHatchDefLine line;

                        line.m_angle   = fbDefLine.angle();
                        line.m_through = *((DPoint2dCP) fbDefLine.through());
                        line.m_offset  = *((DPoint2dCP) fbDefLine.offset());
                        line.m_nDashes = static_cast<short>(fbDefLine.dashes()->Length());

                        if (0 != line.m_nDashes)
                            memcpy(line.m_dashes, fbDefLine.dashes()->Data(), line.m_nDashes * sizeof(double));

                        defLines.push_back(line);
                        }

                    pattern->SetDwgHatchDef(defLines);
                    }

                pattern->ToJson(output.appendValue()["pattern"]);
                break;
                }

            case GeometryStreamIO::OpCode::Material:
                {
                auto ppfb = flatbuffers::GetRoot<FB::Material>(egOp.m_data);
                auto value = output.appendObject()["material"];
                value.SetEmptyObject();

                // NB: Geometry may override uv detail while still using material from subcategory.
                if (ppfb->has_trans2x3())
                    reinterpret_cast<Render::TextureMapping::Trans2x3 const*>(ppfb->trans2x3())->ToJson(value["trans2x3"]);

                if (ppfb->useMaterial())
                    {
                    RenderMaterialId material((uint64_t)ppfb->materialId());
                    value["materialId"] = material.ToHexStr();
                    }

                break;
                }

            case GeometryStreamIO::OpCode::SubGraphicRange:
                {
                DRange3d range;

                if (!reader.Get(egOp, range))
                    break;

                BeJsGeomUtils::DRange3dToJson(output.appendValue()["subRange"], range);
                break;
                }

            case GeometryStreamIO::OpCode::GeometryPartInstance:
                {
                auto ppfb = flatbuffers::GetRoot<FB::GeometryPart>(egOp.m_data);
                auto value = output.appendValue()["geomPart"];

                DgnGeometryPartId partId = DgnGeometryPartId((uint64_t)ppfb->geomPartId());
                value["part"] = partId.ToHexStr();

                if (ppfb->has_origin())
                    BeJsGeomUtils::DPoint3dToJson(value["origin"], *((DPoint3dCP) ppfb->origin()));

                if (ppfb->has_yaw() || ppfb->has_pitch() || ppfb->has_roll())
                    BeJsGeomUtils::YawPitchRollToJson(value["rotation"], YawPitchRollAngles::FromDegrees(ppfb->yaw(), ppfb->pitch(), ppfb->roll()));

                if (ppfb->has_scale())
                    value["scale"] = ppfb->scale();

                break;
                }

            case GeometryStreamIO::OpCode::TextString:
                {
                TextStringPtr text = TextString::Create(m_state.m_dgnDb);
                if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(*text, egOp.m_data, egOp.m_dataSize))
                    break;

                TextStringStyleCR style = text->GetStyle();
                FontId fontId = style.GetFont();
                if (!fontId.IsValid())
                    break; // Shouldn't happen...DecodeFromFlatBuf would have failed...


                auto value = output.appendValue()["textString"];

                // we're going to store the fontid as a 32 bit value, even though in memory we have a 64bit value. Make sure the high bits are 0.
                BeAssert(fontId.GetValue() == (int64_t)((uint32_t)fontId.GetValue()));
                value["font"] = (uint32_t)fontId.GetValue();
                value["text"] = text->GetText().c_str();
                value["height"] = style.GetHeight();

                double widthFactor = (style.GetWidth() / style.GetHeight());
                if (!DoubleOps::AlmostEqual(widthFactor, 1.0))
                    value["widthFactor"] = widthFactor;

                if (style.IsBold())
                    value["bold"] = true;

                if (style.IsItalic())
                    value["italic"] = true;

                if (style.IsUnderlined())
                    value["underline"] = true;

                if (!text->GetOrigin().IsEqual(DPoint3d::FromZero()))
                    BeJsGeomUtils::DPoint3dToJson(value["origin"], text->GetOrigin());

                if (!text->GetOrientation().IsIdentity())
                    {
                    YawPitchRollAngles angles;
                    YawPitchRollAngles::TryFromRotMatrix(angles, text->GetOrientation()); // NOTE: Text orientation should not have scale/skew...can ignore strict Angle::SmallAngle check.
                    BeJsGeomUtils::YawPitchRollToJson(value["rotation"], angles);
                    }

                break;
                }

            case GeometryStreamIO::OpCode::Image:
                {
                ImageGraphicPtr img;
                if (!reader.Get(egOp, img))
                    break;

                img->ToJson(output.appendValue()["image"]);

                break;
                }

            case GeometryStreamIO::OpCode::ParasolidBRep:
                {
                auto ppfb = flatbuffers::GetRoot<FB::BRepData>(egOp.m_data);
                auto value = output.appendValue()["brep"];
                value.SetEmptyObject();
                if (wantBRepData)
                    {
                    Utf8String  brepStr;

                    Base64Utilities::Encode(brepStr, ppfb->entityData()->Data(), ppfb->entityData()->Length(), JsValueRef::base64Header);
                    value["data"] = brepStr.c_str();
                    }

                if (0 != ppfb->brepType())
                    value["type"] = ppfb->brepType();

                Transform  entityTransform = *((TransformCP) ppfb->entityTransform());

                if (!entityTransform.IsIdentity())
                    BeJsGeomUtils::TransformToJson(value["transform"], entityTransform);

                if (ppfb->has_symbology())
                    {
                    // NOTE: Ignoring older breps w/o face attachment index attrib, not worth the hassle...don't want to add it here...
                    if (!ppfb->has_symbologyIndex())
                        {
                        auto array = value["faceSymbology"];
                        for (size_t iSymb=0; iSymb < ppfb->symbology()->Length(); iSymb++)
                            {
                            FB::FaceSymbology const* fbSymb = ((FB::FaceSymbology const*) ppfb->symbology()->Data())+iSymb;
                            auto faceValue = array.appendValue();

                            if (fbSymb->useColor())
                                {
                                faceValue["color"] = fbSymb->color();
                                if (0.0 != fbSymb->transparency())
                                    faceValue["transparency"] = fbSymb->transparency();
                                }

                            if (fbSymb->useMaterial())
                                {
                                RenderMaterialId material((uint64_t)fbSymb->materialId());
                                faceValue["materialId"] = material.ToHexStr();
                                }
                            }
                        }
                    }

                break;
                }

            case GeometryStreamIO::OpCode::PointPrimitive:
            case GeometryStreamIO::OpCode::PointPrimitive2d:
            case GeometryStreamIO::OpCode::ArcPrimitive:
            case GeometryStreamIO::OpCode::CurveVector:
            case GeometryStreamIO::OpCode::Polyface:
            case GeometryStreamIO::OpCode::CurvePrimitive:
            case GeometryStreamIO::OpCode::SolidPrimitive:
            case GeometryStreamIO::OpCode::BsplineSurface:
                {
                GeometricPrimitivePtr geom;

                if (!reader.Get(egOp, geom))
                    break;

                IGeometryPtr geomPtr;

                switch (geom->GetGeometryType())
                    {
                    case GeometricPrimitive::GeometryType::CurvePrimitive:
                        {
                        geomPtr = IGeometry::Create(geom->GetAsICurvePrimitive());
                        break;
                        }

                    case GeometricPrimitive::GeometryType::CurveVector:
                        {
                        geomPtr = IGeometry::Create(geom->GetAsCurveVector());
                        break;
                        }

                    case GeometricPrimitive::GeometryType::SolidPrimitive:
                        {
                        geomPtr = IGeometry::Create(geom->GetAsISolidPrimitive());
                        break;
                        }

                    case GeometricPrimitive::GeometryType::BsplineSurface:
                        {
                        geomPtr = IGeometry::Create(geom->GetAsMSBsplineSurface());
                        break;
                        }

                    case GeometricPrimitive::GeometryType::Polyface:
                        {
                        geomPtr = IGeometry::Create(geom->GetAsPolyfaceHeader());
                        break;
                        }
                    }

                if (!geomPtr.IsValid())
                    break;

                auto value = output.appendValue();
                if (!IModelJson::TryGeometryToIModelJsonValue(value, *geomPtr))
                    break;
                break;
                }

            default:
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool is3dGeometryType(GeometricPrimitive::GeometryType geomType)
    {
    switch (geomType)
        {
        case GeometricPrimitive::GeometryType::SolidPrimitive:
        case GeometricPrimitive::GeometryType::BsplineSurface:
        case GeometricPrimitive::GeometryType::Polyface:
        case GeometricPrimitive::GeometryType::BRepEntity:
            return true;

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryStreamEntryId GeometryBuilder::GetGeometryStreamEntryId() const
    {
    GeometryStreamEntryId entryId;

    GeometryStreamIO::Collection collection(&m_writer.m_buffer.front(), (uint32_t) m_writer.m_buffer.size());

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::GeometryPartInstance:
                {
                auto ppfb = flatbuffers::GetRoot<FB::GeometryPart>(egOp.m_data);

                entryId.SetGeometryPartId(DgnGeometryPartId((uint64_t)ppfb->geomPartId()));
                entryId.IncrementIndex();
                break;
                }

            default:
                {
                if (!egOp.IsGeometryOp())
                    break;

                entryId.SetGeometryPartId(DgnGeometryPartId());
                entryId.IncrementIndex();
                break;
                }
            }
        }

    return entryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometryBuilder::GetGeometryStream(GeometryStreamR geom)
    {
    if (0 == m_writer.m_buffer.size())
        return ERROR;

    geom.SaveData(&m_writer.m_buffer.front(), (uint32_t) m_writer.m_buffer.size());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometryBuilder::Finish(DgnGeometryPartR part)
    {
    if (!m_isPartCreate)
        return ERROR; // Invalid builder for creating part geometry...

    if (0 == m_writer.m_buffer.size())
        return ERROR;

    part.GetGeometryStreamR().SaveData(&m_writer.m_buffer.front(), (uint32_t) m_writer.m_buffer.size());

    ElementAlignedBox3d localRange = (m_is3d ? m_placement3d.GetElementBox() : ElementAlignedBox3d(m_placement2d.GetElementBox()));

    // NOTE: GeometryBuilder::CreateGeometryPart doesn't supply range...need to compute it...
    if (!localRange.IsValid())
        {
        GeometryCollection collection(part.GetGeometryStream(), m_dgnDb);

        for (auto iter : collection)
            {
            GeometricPrimitivePtr geom = iter.GetGeometryPtr();
            DRange3d range;

            if (geom.IsValid() && geom->GetRange(range))
                {
                localRange.Extend(range);
                }
            }

        if (!localRange.IsValid())
            return ERROR;
        }

    part.SetBoundingBox(localRange);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometryBuilder::Finish(GeometrySourceR source)
    {
    if (m_isPartCreate)
        return ERROR; // Invalid builder for creating element geometry...

    if (0 == m_writer.m_buffer.size())
        return ERROR;

    if (!m_havePlacement)
        return ERROR;

    if (source.GetCategoryId() != m_elParams.GetCategoryId())
        return ERROR;

    if (m_is3d)
        {
        if (!m_placement3d.IsValid())
            return ERROR;

        GeometrySource3dP source3d;

        if (nullptr == (source3d = source.GetAsGeometrySource3dP()))
            return ERROR;

        source3d->SetPlacement(m_placement3d);
        }
    else
        {
        if (!m_placement2d.IsValid())
            return ERROR;

        GeometrySource2dP source2d;

        if (nullptr == (source2d = source.GetAsGeometrySource2dP()))
            return ERROR;

        source2d->SetPlacement(m_placement2d);
        }

    source.GetGeometryStreamR().SaveData(&m_writer.m_buffer.front(), (uint32_t) m_writer.m_buffer.size());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometryBuilder::ClearGeometryStream(GeometrySourceR source)
    {
    if (m_isPartCreate)
        return ERROR; // Invalid for parts...

    if (m_is3d)
        {
        GeometrySource3dP source3d;

        if (nullptr == (source3d = source.GetAsGeometrySource3dP()))
            return ERROR;

        Placement3d placement3d = source3d->GetPlacement();
        
        placement3d.GetElementBoxR() = ElementAlignedBox3d();
        source3d->SetPlacement(placement3d);
        }
    else
        {
        GeometrySource2dP source2d;

        if (nullptr == (source2d = source.GetAsGeometrySource2dP()))
            return ERROR;

        Placement2d placement2d = source2d->GetPlacement();
        
        placement2d.GetElementBoxR() = ElementAlignedBox2d();
        source2d->SetPlacement(placement2d);
        }

    source.GetGeometryStreamR().Clear();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(DgnSubCategoryId subCategoryId)
    {
    GeometryParams elParams;

    elParams.SetCategoryId(m_elParams.GetCategoryId()); // Preserve current category...
    elParams.SetSubCategoryId(subCategoryId);

    return Append(elParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(GeometryParamsCR elParams, CoordSystem coord)
    {
    // NOTE: Allow explicit symbology in GeometryPart's GeometryStream, sub-category won't be persisted...
    if (!m_isPartCreate)
        {
        if (!m_elParams.GetCategoryId().IsValid())
            return false;

        if (elParams.GetCategoryId() != m_elParams.GetCategoryId())
            return false;

        if (elParams.GetCategoryId() != DgnSubCategory::QueryCategoryId(m_dgnDb, elParams.GetSubCategoryId()))
            return false;
        }

    if (elParams.IsTransformable())
        {
        if (CoordSystem::World == coord)
            {
            if (m_isPartCreate)
                {
                BeAssert(false); // Part GeometryParams must be supplied in local coordinates...
                return false;
                }

            // NOTE: Must defer applying transform until placement is computed from first geometric primitive...
            if (m_havePlacement)
                {
                Transform   localToWorld = (m_is3d ? m_placement3d.GetTransform() : m_placement2d.GetTransform());
                Transform   worldToLocal;

                worldToLocal.InverseOf(localToWorld);

                if (!worldToLocal.IsIdentity())
                    {
                    GeometryParams localParams(elParams);

                    localParams.ApplyTransform(worldToLocal);

                    return Append(localParams, CoordSystem::Local);
                    }
                }
            }
        else
            {
            if (!m_havePlacement)
                {
                BeAssert(false); // Caller can't supply local coordinates if we don't know what local is yet...
                return false;
                }
            }
        }

    if (m_elParams.IsEquivalent(elParams))
        return true;

    m_subCategoryChanged = (!m_isPartCreate && (elParams.GetSubCategoryId() != m_elParams.GetSubCategoryId()));
    m_elParams = elParams;
    m_appearanceChanged = true; // Defer append until we actually have some geometry...

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(DgnGeometryPartId geomPartId, TransformCR geomToElement, DRange3dCR localRange)
    {
    if (m_isPartCreate)
        {
        BeAssert(false); // Nested parts are not supported...
        return false;
        }

    if (!m_havePlacement)
        return false; // geomToElement must be relative to an already defined placement (i.e. not computed placement from CreateWorld)...

    DRange3d    partRange = localRange;

    if (!geomToElement.IsIdentity())
        geomToElement.Multiply(partRange, partRange);

    OnNewGeom(partRange, false, GeometryStreamIO::OpCode::GeometryPartInstance); // Parts are already handled as sub-graphics...
    m_writer.Append(geomPartId, &geomToElement);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(DgnGeometryPartId geomPartId, TransformCR geomToElement)
    {
    DRange3d    localRange;

    if (SUCCESS != DgnGeometryPart::QueryGeometryPartRange(localRange, m_dgnDb, geomPartId))
        return false; // part probably doesn't exist...

    return Append(geomPartId, geomToElement, localRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(DgnGeometryPartId geomPartId, DPoint3dCR origin, YawPitchRollAngles const& angles)
    {
    Transform   geomToElement = angles.ToTransform(origin);

    return Append(geomPartId, geomToElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(DgnGeometryPartId geomPartId, DPoint2dCR origin, AngleInDegrees const& angle)
    {
    Transform   geomToElement;

    geomToElement.InitFromOriginAngleAndLengths(origin, angle.Radians(), 1.0, 1.0);

    return Append(geomPartId, geomToElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryBuilder::OnNewGeom(DRange3dCR localRangeIn, bool isSubGraphic, GeometryStreamIO::OpCode opCode)
    {
    DRange3d localRange = localRangeIn;

    // NOTE: Expand range to include line style width.
    if (m_elParams.GetCategoryId().IsValid())
        {
        m_elParams.Resolve(m_dgnDb);

        LineStyleInfoCP lsInfo = m_elParams.GetLineStyle();

        if (nullptr != lsInfo)
            {
            double maxWidth = lsInfo->GetLineStyleSymb().GetStyleWidth();

            localRange.low.x -= maxWidth;
            localRange.low.y -= maxWidth;
            localRange.high.x += maxWidth;
            localRange.high.y += maxWidth;

            if (m_is3d)
                {
                localRange.low.z -= maxWidth;
                localRange.high.z += maxWidth;
                }
            }
        }

    if (m_is3d)
        m_placement3d.GetElementBoxR().Extend(localRange);
    else
        m_placement2d.GetElementBoxR().Extend(DRange2d::From(DPoint2d::From(localRange.low), DPoint2d::From(localRange.high)));

    bool allowPatGradnt = false;
    bool allowSolidFill = false;
    bool allowLineStyle = false;
    bool allowMaterial  = false;

    switch (opCode)
        {
        case GeometryStreamIO::OpCode::GeometryPartInstance:
            allowSolidFill = allowPatGradnt = allowLineStyle = allowMaterial = true; // Don't reject anything...
            break;

        case GeometryStreamIO::OpCode::CurvePrimitive:
            allowLineStyle = true;
            break;

        case GeometryStreamIO::OpCode::CurveVector:
            allowSolidFill = allowPatGradnt = allowLineStyle = allowMaterial = true;
            break;

        case GeometryStreamIO::OpCode::Polyface:
            allowSolidFill = allowMaterial = true;
            break;

        case GeometryStreamIO::OpCode::SolidPrimitive:
        case GeometryStreamIO::OpCode::BsplineSurface:
        case GeometryStreamIO::OpCode::ParasolidBRep:
            allowMaterial = true;
            break;

        case GeometryStreamIO::OpCode::Image:
            allowSolidFill = true;
            break;
        }

    bool hasInvalidPatGradnt = false;
    bool hasInvalidSolidFill = false;
    bool hasInvalidLineStyle = false;
    bool hasInvalidMaterial  = false;

    if (!allowPatGradnt || !allowSolidFill || !allowLineStyle || !allowMaterial)
        {
        if (FillDisplay::Never != m_elParams.GetFillDisplay())
            {
            if (nullptr != m_elParams.GetGradient() && !m_elParams.GetGradient()->IsThematic())
                {
                if (!allowPatGradnt)
                    hasInvalidPatGradnt = true;
                }
            else
                {
                if (!allowSolidFill)
                    hasInvalidSolidFill = true;
                }
            }

        if (!allowPatGradnt && nullptr != m_elParams.GetPatternParams())
            hasInvalidPatGradnt = true;

        if (!allowLineStyle && !m_elParams.IsLineStyleFromSubCategoryAppearance() && m_elParams.HasStrokedLineStyle())
            hasInvalidLineStyle = true;

        if (!allowMaterial && !m_elParams.IsMaterialFromSubCategoryAppearance() && m_elParams.GetMaterialId().IsValid())
            hasInvalidMaterial = true;
        }

    if (hasInvalidPatGradnt || hasInvalidSolidFill || hasInvalidLineStyle || hasInvalidMaterial)
        {
        // NOTE: We won't change m_elParams in case some caller is doing something like appending a single symbology
        //       that includes fill, and then adding a series of open and closed elements expecting the open elements
        //       to ignore the fill.
        GeometryParams localParams(m_elParams);

        if (hasInvalidPatGradnt)
            {
            localParams.SetGradient(nullptr);
            localParams.SetPatternParams(nullptr);
            }

        if (hasInvalidSolidFill || hasInvalidPatGradnt)
            localParams.SetFillDisplay(FillDisplay::Never);

        if (hasInvalidLineStyle)
            localParams.SetLineStyle(nullptr);

        if (hasInvalidMaterial)
            localParams.SetMaterialId(RenderMaterialId());

        if (!m_appearanceModified || !m_elParamsModified.IsEquivalent(localParams))
            {
            m_elParamsModified = localParams;
            m_writer.Append(m_elParamsModified, m_isPartCreate || !m_subCategoryChanged, m_is3d);
            m_appearanceChanged = m_appearanceModified = true;
            m_subCategoryChanged = false;
            }
        }
    else if (m_appearanceChanged)
        {
        m_writer.Append(m_elParams, m_isPartCreate || !m_subCategoryChanged, m_is3d);
        m_appearanceChanged = m_appearanceModified = m_subCategoryChanged = false;
        }

    if (isSubGraphic && !m_isPartCreate)
        m_writer.Append(localRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::ConvertToLocal(GeometricPrimitiveR geom)
    {
    if (m_isPartCreate)
        {
        BeAssert(false); // Part geometry must be supplied in local coordinates...
        return false;
        }

    Transform   localToWorld;
    bool        transformParams = !m_havePlacement;

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

    // NOTE: Apply world-to-local to GeometryParams for auto-placement data supplied in world coords...
    if (transformParams && m_elParams.IsTransformable())
        m_elParams.ApplyTransform(worldToLocal);

    return geom.TransformInPlace(worldToLocal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::AppendLocal(GeometricPrimitiveCR geom)
    {
    if (!m_havePlacement)
        {
        BeAssert(false); // placement must already be defined...
        return false;
        }

    DRange3d localRange;

    if (!geom.GetRange(localRange))
        return false;

    GeometryStreamIO::OpCode opCode;

    switch (geom.GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurvePrimitive:
            opCode = GeometryStreamIO::OpCode::CurvePrimitive;
            break;

        case GeometricPrimitive::GeometryType::CurveVector:
            opCode = geom.GetAsCurveVector()->IsAnyRegionType() ? GeometryStreamIO::OpCode::CurveVector : GeometryStreamIO::OpCode::CurvePrimitive;
            break;

        case GeometricPrimitive::GeometryType::SolidPrimitive:
            opCode = GeometryStreamIO::OpCode::SolidPrimitive;
            break;

        case GeometricPrimitive::GeometryType::BsplineSurface:
            opCode = GeometryStreamIO::OpCode::BsplineSurface;
            break;

        case GeometricPrimitive::GeometryType::Polyface:
            opCode = GeometryStreamIO::OpCode::Polyface;
            break;

        case GeometricPrimitive::GeometryType::BRepEntity:
            opCode = GeometryStreamIO::OpCode::ParasolidBRep;
            break;

        case GeometricPrimitive::GeometryType::TextString:
            opCode = GeometryStreamIO::OpCode::TextString;
            break;

        case GeometricPrimitive::GeometryType::Image:
            opCode = GeometryStreamIO::OpCode::Image;
            break;

        default:
            opCode = GeometryStreamIO::OpCode::Invalid;
            break;
        }

    OnNewGeom(localRange, m_appendAsSubGraphics, opCode);

    return m_writer.AppendSimplified(geom, m_is3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::AppendWorld(GeometricPrimitiveR geom)
    {
    if (!ConvertToLocal(geom))
        return false;

    return AppendLocal(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(GeometricPrimitiveCR geom, CoordSystem coord)
    {
    if (!m_is3d && is3dGeometryType(geom.GetGeometryType()))
        {
        BeAssert(false); // 3d only geometry...
        return false;
        }

    if (CoordSystem::Local == coord)
        return AppendLocal(geom);

    GeometricPrimitivePtr clone;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (GeometricPrimitive::GeometryType::BRepEntity == geom.GetGeometryType())
        {
        IBRepEntityPtr instance = geom.GetAsIBRepEntity()->CreateInstance();

        if (!instance.IsValid())
            return false;

        clone = GeometricPrimitive::Create(instance);
        }
    else
        {
        clone = geom.Clone();
        }

    return AppendWorld(*clone);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(ICurvePrimitiveCR geom, CoordSystem coord)
    {
    if (CoordSystem::Local == coord)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange, m_appendAsSubGraphics, GeometryStreamIO::OpCode::CurvePrimitive);

        return m_writer.AppendSimplified(geom, false, m_is3d);
        }

    GeometricPrimitivePtr geomPtr = GeometricPrimitive::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(CurveVectorCR geom, CoordSystem coord)
    {
    if (CoordSystem::Local == coord)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange, m_appendAsSubGraphics, geom.IsAnyRegionType() ? GeometryStreamIO::OpCode::CurveVector : GeometryStreamIO::OpCode::CurvePrimitive);

        return m_writer.AppendSimplified(geom, m_is3d);
        }

    GeometricPrimitivePtr geomPtr = GeometricPrimitive::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(ISolidPrimitiveCR geom, CoordSystem coord)
    {
    if (!m_is3d)
        {
        BeAssert(false); // 3d only geometry...
        return false;
        }

    if (CoordSystem::Local == coord)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange, m_appendAsSubGraphics, GeometryStreamIO::OpCode::SolidPrimitive);
        m_writer.Append(geom);

        return true;
        }

    GeometricPrimitivePtr geomPtr = GeometricPrimitive::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(MSBsplineSurfaceCR geom, CoordSystem coord)
    {
    if (!m_is3d)
        {
        BeAssert(false); // 3d only geometry...
        return false;
        }

    if (CoordSystem::Local == coord)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange, m_appendAsSubGraphics, GeometryStreamIO::OpCode::BsplineSurface);
        m_writer.Append(geom);

        return true;
        }

    GeometricPrimitivePtr geomPtr = GeometricPrimitive::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(PolyfaceQueryCR geom, CoordSystem coord)
    {
    if (!m_is3d)
        {
        BeAssert(false); // 3d only geometry...
        return false;
        }

    if (CoordSystem::Local == coord)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange, m_appendAsSubGraphics, GeometryStreamIO::OpCode::Polyface);
        m_writer.Append(geom);

        return true;
        }

    GeometricPrimitivePtr geomPtr = GeometricPrimitive::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(IGeometryCR geometry, CoordSystem coord)
    {
    switch (geometry.GetGeometryType())
        {
        case IGeometry::GeometryType::CurvePrimitive:
            return Append(*geometry.GetAsICurvePrimitive(), coord);
        case IGeometry::GeometryType::CurveVector:
            return Append(*geometry.GetAsCurveVector(), coord);
        case IGeometry::GeometryType::Polyface:
            return Append(*geometry.GetAsPolyfaceHeader(), coord);
        case IGeometry::GeometryType::SolidPrimitive:
            return Append(*geometry.GetAsISolidPrimitive(), coord);
        case IGeometry::GeometryType::BsplineSurface:
            return Append(*geometry.GetAsMSBsplineSurface(), coord);
        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(IBRepEntityCR geom, CoordSystem coord)
    {
    if (!m_is3d)
        {
        BeAssert(false); // 3d only geometry...
        return false;
        }

    if (CoordSystem::Local == coord)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange, m_appendAsSubGraphics, GeometryStreamIO::OpCode::ParasolidBRep);
        m_writer.Append(geom);

        return true;
        }

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    IBRepEntityPtr instance = geom.CreateInstance();

    if (!instance.IsValid())
        return false;

    GeometricPrimitivePtr geomPtr = GeometricPrimitive::Create(instance);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(TextStringCR text, CoordSystem coord)
    {
    if (CoordSystem::Local == coord)
        {
        DRange3d localRange;
        if (!getRange(text, localRange, nullptr))
            return false;

        OnNewGeom(localRange, m_appendAsSubGraphics, GeometryStreamIO::OpCode::TextString);
        m_writer.Append(text);

        return true;
        }

    return AppendWorld(*GeometricPrimitive::Create(text));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(ImageGraphicCR img, CoordSystem coord)
    {
    if (CoordSystem::Local == coord)
        {
        DRange3d localRange;
        if (!getRange(img, localRange, nullptr))
            return false;

        OnNewGeom(localRange, m_appendAsSubGraphics, GeometryStreamIO::OpCode::Image);
        m_writer.Append(img);

        return true;
        }

    auto geom = GeometricPrimitive::Create(img);
    return AppendWorld(*geom);
    }

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
//! Allows for the ViewContext-based TextAnnotationDraw to be used with an GeometryBuilder.
// @bsiclass
//=======================================================================================
struct TextAnnotationDrawToGeometricPrimitive : IGeometryProcessor
{
private:
    GeometryBuilderR m_builder;
    TextAnnotationCR m_text;
    GeometryBuilder::CoordSystem m_coord;

public:
    TextAnnotationDrawToGeometricPrimitive(TextAnnotationCR text, GeometryBuilderR builder, GeometryBuilder::CoordSystem coord) : m_text(text), m_builder(builder), m_coord(coord) {}

    bool _ProcessTextString(TextStringCR, SimplifyGraphic&) override;
    bool _ProcessCurveVector(CurveVectorCR, bool isFilled, SimplifyGraphic&) override;
    void _OutputGraphics(ViewContextR) override;
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool TextAnnotationDrawToGeometricPrimitive::_ProcessTextString(TextStringCR text, SimplifyGraphic& graphic)
    {
    m_builder.Append(graphic.GetCurrentGeometryParams()); // NOTE: Actual append happens when geometric primitive is added only if changed...

    if (graphic.GetLocalToWorldTransform().IsIdentity())
        {
        m_builder.Append(text, m_coord);

        return true;
        }

    TextString transformedText(text);

    transformedText.ApplyTransform(graphic.GetLocalToWorldTransform());
    m_builder.Append(transformedText, m_coord);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool TextAnnotationDrawToGeometricPrimitive::_ProcessCurveVector(CurveVectorCR curves, bool isFilled, SimplifyGraphic& graphic)
    {
    m_builder.Append(graphic.GetCurrentGeometryParams()); // NOTE: Actual append happens when geometric primitive is added only if changed...

    if (graphic.GetLocalToWorldTransform().IsIdentity())
        {
        m_builder.Append(curves, m_coord);

        return true;
        }

    CurveVector transformedCurves(curves);

    transformedCurves.TransformInPlace(graphic.GetLocalToWorldTransform());
    m_builder.Append(transformedCurves, m_coord);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextAnnotationDrawToGeometricPrimitive::_OutputGraphics(ViewContextR context)
    {
    TextAnnotationDraw annotationDraw(m_text);
    Render::GeometryParams geomParams(m_builder.GetGeometryParams());
    auto graphic = context.CreateSceneGraphic();

    annotationDraw.Draw(*graphic, context, geomParams);
    graphic->Finish();
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(TextAnnotationCR text, CoordSystem coord)
    {
    TextAnnotationDrawToGeometricPrimitive annotationDraw(text, *this, coord);

    GeometryProcessor::Process(annotationDraw, m_dgnDb);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::SetHeaderFlags(GeometryStreamIO::Header::Flags flags)
    {
    if (m_isPartCreate)
        {
        BeAssert(false); // Not valid when creating a part...
        return false;
        }

    m_writer.Reset(flags);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilder::GeometryBuilder(DgnDbR dgnDb, DgnCategoryId categoryId, Placement3dCR placement)
  : GeometryBuilder(dgnDb, categoryId, true)
    {
    m_placement3d = placement;
    m_placement3d.GetElementBoxR().Init(); // throw away pre-existing bounding box...
    m_havePlacement = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilder::GeometryBuilder(DgnDbR dgnDb, DgnCategoryId categoryId, Placement2dCR placement)
  : GeometryBuilder(dgnDb, categoryId, false)
    {
    m_placement2d = placement;
    m_placement2d.GetElementBoxR().Init(); // throw away pre-existing bounding box...
    m_havePlacement = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilder::GeometryBuilder(DgnDbR dgnDb, DgnCategoryId categoryId, bool is3d)
  : m_isPartCreate(!categoryId.IsValid()), m_is3d(is3d), m_dgnDb(dgnDb), m_writer(dgnDb)
    {
    m_elParams.SetCategoryId(categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr GeometryBuilder::CreateGeometryPart(GeometryStreamCR stream, DgnDbR db, bool ignoreSymbology, Render::GeometryParamsP params)
    {
    GeometryBuilderPtr builder = new GeometryBuilder(db, DgnCategoryId(), Placement3d());
    GeometryStreamIO::Collection collection(stream.GetData(), stream.GetSize());
    GeometryStreamIO::Reader reader(db);
    size_t basicSymbCount = 0;

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                break; // Already have header....

            case GeometryStreamIO::OpCode::SubGraphicRange:
                break; // A part must produce a single graphic...

            case GeometryStreamIO::OpCode::GeometryPartInstance:
                return nullptr; // Nested parts aren't supported...

            case GeometryStreamIO::OpCode::BasicSymbology:
                {
                if (ignoreSymbology)
                    break;

                basicSymbCount++;

                if (1 == basicSymbCount && nullptr != params)
                    {
                    reader.Get(egOp, *params);
                    break; // Initial symbology should not be baked into GeometryPart...
                    }

                // Can't change sub-category in GeometryPart's GeometryStream, only preserve sub-category appearance overrides.
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
                builder->m_writer.Append(GeometryStreamIO::Operation(GeometryStreamIO::OpCode::BasicSymbology, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
                break;
                }

            case GeometryStreamIO::OpCode::LineStyleModifiers:
            case GeometryStreamIO::OpCode::AreaFill:
            case GeometryStreamIO::OpCode::Pattern:
            case GeometryStreamIO::OpCode::Material:
                {
                if (ignoreSymbology)
                    break;

                if (1 == basicSymbCount && nullptr != params)
                    {
                    reader.Get(egOp, *params);
                    break; // Initial symbology should not be baked into GeometryPart...
                    }

                builder->m_writer.Append(egOp); // Append raw data...
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr GeometryBuilder::CreateGeometryPart(DgnDbR db, bool is3d)
    {
    // NOTE: Part geometry is always specified in local coords, i.e. has identity placement.
    //       Category isn't needed when creating a part, invalid category will be used to set m_isPartCreate.
    if (is3d)
        return new GeometryBuilder(db, DgnCategoryId(), Placement3d());

    return new GeometryBuilder(db, DgnCategoryId(), Placement2d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr GeometryBuilder::Create(GeometrySourceCR source, GeometryStreamCR stream)
    {
    DgnCategoryId categoryId = source.GetCategoryId();

    if (!categoryId.IsValid())
        return nullptr;

    GeometryBuilderPtr builder;

    if (nullptr != source.GetAsGeometrySource3d())
        builder = new GeometryBuilder(source.GetSourceDgnDb(), categoryId, source.GetAsGeometrySource3d()->GetPlacement());
    else
        builder = new GeometryBuilder(source.GetSourceDgnDb(), categoryId, source.GetAsGeometrySource2d()->GetPlacement());

    if (!builder.IsValid())
        return nullptr;

    Render::GeometryParams sourceParams(categoryId);
    Transform sourceTransform = source.GetPlacementTransform();

    GeometryCollection collection(stream, source.GetSourceDgnDb(), &sourceParams, &sourceTransform);

    for (auto iter : collection)
        {
        builder->Append(iter.GetGeometryParams());

        DgnGeometryPartId partId = iter.GetGeometryPartId();

        if (partId.IsValid())
            {
            builder->Append(partId, iter.GetGeometryToSource());
            continue;
            }

        GeometricPrimitivePtr geom = iter.GetGeometryPtr();

        BeAssert(geom.IsValid());
        if (!geom.IsValid())
            continue;

        if (!iter.GetSubGraphicLocalRange().IsNull())
            builder->SetAppendAsSubGraphics();

        builder->Append(*geom);
        }

    return builder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr GeometryBuilder::CreateWithAutoPlacement(DgnModelR model, DgnCategoryId categoryId)
    {
    if (!categoryId.IsValid())
        return nullptr;

    auto geomModel = model.ToGeometricModel();
    if (nullptr == geomModel)
        return nullptr;

    return new GeometryBuilder(model.GetDgnDb(), categoryId, geomModel->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr GeometryBuilder::Create(DgnModelR model, DgnCategoryId categoryId, TransformCR transform)
    {
    if (!categoryId.IsValid())
        return nullptr;

    auto geomModel = model.ToGeometricModel();
    if (nullptr == geomModel)
        return nullptr;

    DPoint3d            origin;
    RotMatrix           rMatrix;
    YawPitchRollAngles  angles;

    transform.GetTranslation(origin);
    transform.GetMatrix(rMatrix);

    // NOTE: YawPitchRollAngles::TryFromRotMatrix compares against Angle::SmallAngle, which
    //       is too strict for our purposes and shouldn't be considered a failure.
    if (!YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix))
        {
        RotMatrix   resultMatrix = angles.ToRotMatrix();

        if (rMatrix.MaxDiff(resultMatrix) > 1.0e-5)
            return nullptr;
        }

    if (geomModel->Is3d())
        {
        Placement3d placement(origin, angles);

        return new GeometryBuilder(model.GetDgnDb(), categoryId, placement);
        }

    if (0 != BeNumerical::Compare(origin.z, 0.0) || 0 != BeNumerical::Compare(angles.GetPitch().Degrees(), 0.0) || 0 != BeNumerical::Compare(angles.GetRoll().Degrees(), 0.0))
        return nullptr; // Invalid transform for Placement2d...

    Placement2d placement(DPoint2d::From(origin), angles.GetYaw());

    return new GeometryBuilder(model.GetDgnDb(), categoryId, placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr GeometryBuilder::Create(DgnModelR model, DgnCategoryId categoryId, DPoint3dCR origin, YawPitchRollAngles const& angles)
    {
    if (!categoryId.IsValid())
        return nullptr;

    auto geomModel = model.ToGeometricModel();
    if (nullptr == geomModel || !geomModel->Is3d())
        return nullptr;

    Placement3d placement(origin, angles);

    return new GeometryBuilder(model.GetDgnDb(), categoryId, placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr GeometryBuilder::Create(DgnModelR model, DgnCategoryId categoryId, DPoint2dCR origin, AngleInDegrees const& angle)
    {
    if (!categoryId.IsValid())
        return nullptr;

    auto geomModel = model.ToGeometricModel();
    if (nullptr == geomModel || geomModel->Is3d())
        return nullptr;

    Placement2d placement(origin, angle);

    return new GeometryBuilder(model.GetDgnDb(), categoryId, placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr GeometryBuilder::Create(GeometrySourceCR source)
    {
    DgnCategoryId categoryId = source.GetCategoryId();

    if (!categoryId.IsValid())
        return nullptr;

    if (nullptr != source.GetAsGeometrySource3d())
        return new GeometryBuilder(source.GetSourceDgnDb(), categoryId, source.GetAsGeometrySource3d()->GetPlacement());

    return new GeometryBuilder(source.GetSourceDgnDb(), categoryId, source.GetAsGeometrySource2d()->GetPlacement());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::FromJson(BeJsConst input)
    {
    Render::GeometryParams params = GetGeometryParams();
    int n = input.size();
    bool checkedSubGraphics = false;

    for (int i = 0; i < n; i++)
        {
        auto entry = input[i];

        if (!entry.isObject())
            continue;

        if (entry.isMember("header"))
            {
            BeAssert(0 == i);
            m_writer.Reset(static_cast<GeometryStreamIO::Header::Flags>(entry["header"]["flags"].asUInt()));
            }
        else if (entry.isMember("appearance"))
            {
            auto appearance = entry["appearance"];

            params.ResetAppearance(); // Always reset to subCategory appearance...

            if (!appearance["subCategory"].isNull())
                {
                DgnSubCategoryId subCategoryId;

                subCategoryId.FromJson(appearance["subCategory"]);

                if (subCategoryId.IsValid())
                    params.SetSubCategoryId(subCategoryId);
                }

            if (!appearance["color"].isNull())
                params.SetLineColor(ColorDef(appearance["color"].asUInt()));

            if (!appearance["weight"].isNull())
                params.SetWeight(appearance["weight"].asUInt());

            if (!appearance["style"].isNull())
                {
                DgnStyleId styleId;
                styleId.FromJson(appearance["style"]);

                if (styleId.IsValid())
                    {
                    LineStyleInfoPtr lsInfo = LineStyleInfo::Create(styleId, nullptr);
                    params.SetLineStyle(lsInfo.get());
                    }
                else
                    {
                    params.SetLineStyle(nullptr);
                    }
                }

            if (!appearance["transparency"].isNull())
                params.SetTransparency(appearance["transparency"].asDouble());

            if (!appearance["displayPriority"].isNull())
                params.SetDisplayPriority(appearance["displayPriority"].asInt());

            if (!appearance["geometryClass"].isNull())
                params.SetGeometryClass((Render::DgnGeometryClass) (appearance["geometryClass"].asUInt()));
            }
        else if (entry.isMember("styleMod"))
            {
            auto styleMod = entry["styleMod"];
            LineStyleParams styleParams;

            styleParams.Init();

            if (!styleMod["scale"].isNull())
                {
                styleParams.scale = styleMod["scale"].asDouble();
                styleParams.modifiers |= STYLEMOD_SCALE;
                }

            if (!styleMod["dashScale"].isNull())
                {
                styleParams.dashScale = styleMod["dashScale"].asDouble();
                styleParams.modifiers |= STYLEMOD_DSCALE;
                }

            if (!styleMod["gapScale"].isNull())
                {
                styleParams.gapScale = styleMod["gapScale"].asDouble();
                styleParams.modifiers |= STYLEMOD_GSCALE;
                }

            if (!styleMod["startWidth"].isNull())
                {
                styleParams.startWidth = styleMod["startWidth"].asDouble();
                styleParams.modifiers |= STYLEMOD_SWIDTH;
                }

            if (!styleMod["endWidth"].isNull())
                {
                styleParams.endWidth = styleMod["endWidth"].asDouble();
                styleParams.modifiers |= STYLEMOD_EWIDTH;
                }

            if (!styleMod["distPhase"].isNull())
                {
                styleParams.distPhase = styleMod["distPhase"].asDouble();
                styleParams.modifiers |= STYLEMOD_DISTPHASE;
                }

            if (!styleMod["fractPhase"].isNull())
                {
                styleParams.fractPhase = styleMod["fractPhase"].asDouble();
                styleParams.modifiers |= STYLEMOD_FRACTPHASE;
                }

            if (!styleMod["centerPhase"].isNull())
                styleParams.modifiers |= STYLEMOD_CENTERPHASE;

            if (!styleMod["segmentMode"].isNull())
                {
                if (styleMod["segmentMode"].asBool())
                    styleParams.modifiers |= STYLEMOD_SEGMODE;
                else
                    styleParams.modifiers |= STYLEMOD_NOSEGMODE;
                }

            if (!styleMod["physicalWidth"].isNull())
                styleParams.modifiers |= STYLEMOD_TRUE_WIDTH;

            if (!styleMod["normal"].isNull())
                {
                BeJsGeomUtils::DPoint3dFromJson(styleParams.normal, styleMod["normal"]);
                styleParams.modifiers |= STYLEMOD_NORMAL;
                }

            if (!styleMod["rotation"].isNull())
                {
                YawPitchRollAngles angles = BeJsGeomUtils::YawPitchRollFromJson(styleMod["rotation"]);

                styleParams.rMatrix = angles.ToRotMatrix();
                styleParams.modifiers |= STYLEMOD_RMATRIX;
                }

            DgnStyleId      styleId;
            LineStyleInfoCP currentLsInfo = params.GetLineStyle();

            if (nullptr != currentLsInfo)
                styleId = currentLsInfo->GetStyleId();

            LineStyleInfoPtr lsInfo = LineStyleInfo::Create(styleId, &styleParams);
            params.SetLineStyle(lsInfo.get());
            }
        else if (entry.isMember("fill"))
            {
            auto fill = entry["fill"];

            if (fill["display"].isNull())
                continue;

            FillDisplay fillDisplay = (FillDisplay) fill["display"].asUInt();

            params.SetFillDisplay(fillDisplay);

            if (FillDisplay::Never == fillDisplay)
                continue;

            if (!fill["transparency"].isNull())
                params.SetFillTransparency(fill["transparency"].asDouble());

            if (!fill["gradient"].isNull())
                {
                GradientSymbPtr gradientPtr = GradientSymb::Create();

                if (SUCCESS == gradientPtr->FromJson(fill["gradient"]))
                    {
                    params.SetGradient(gradientPtr.get());
                    continue;
                    }
                }

            if (!fill["backgroundFill"].isNull())
                {
                BackgroundFill bgFill = (BackgroundFill) fill["backgroundFill"].asUInt();

                if (BackgroundFill::None == bgFill)
                    {
                    if (!fill["color"].isNull())
                        params.SetFillColor(ColorDef(fill["color"].asUInt()));
                    else
                        params.SetFillColorToSubCategoryAppearance();
                    }
                else
                    {
                    params.SetFillColorFromViewBackground(BackgroundFill::Outline == bgFill);
                    }
                }
            else if (!fill["color"].isNull())
                {
                params.SetFillColor(ColorDef(fill["color"].asUInt()));
                }
            }
        else if (entry.isMember("pattern"))
            {
            PatternParamsPtr patternParams = PatternParams::Create();

            if (SUCCESS == patternParams->FromJson(entry["pattern"]))
                params.SetPatternParams(patternParams.get());
            }
        else if (entry.isMember("material"))
            {
            auto material = entry["material"];

            if (material["materialId"].isNull())
                return false; // A material id (could be invalid to override sub-cateogory with none) is required...

            RenderMaterialId materialId;
            materialId.FromJson(material["materialId"]);

            if (material["trans2x3"].isArray())
                {
                Render::MaterialUVDetail detail;
                detail.SetTransform(Render::TextureMapping::Trans2x3::FromJson(material["trans2x3"]));
                params.SetMaterialUVDetail(detail);
                }

            params.SetMaterialId(materialId);
            }
        else if (entry.isMember("geomPart"))
            {
            auto geomPart = entry["geomPart"];

            if (geomPart["part"].isNull())
                return false; // A part id is required...

            DgnGeometryPartId partId;
            partId.FromJson(geomPart["part"]);
            if (!partId.IsValid())
                return false;

            DPoint3d origin = DPoint3d::FromZero();
            if (!geomPart["origin"].isNull())
                BeJsGeomUtils::DPoint3dFromJson(origin, geomPart["origin"]);

            YawPitchRollAngles angles;
            if (!geomPart["rotation"].isNull())
                angles = BeJsGeomUtils::YawPitchRollFromJson(geomPart["rotation"]);

            Transform geomToSource = angles.ToTransform(origin);

            if (!geomPart["scale"].isNull())
                {
                double scale = geomPart["scale"].asDouble();
                if (scale > 0.0 && 1.0 != scale)
                    geomToSource.ScaleMatrixColumns(geomToSource, scale, scale, scale);
                }

            if (!Append(params))
                return false;

            if (!Append(partId, geomToSource))
                return false;
            }
        else if (entry.isMember("subRange"))
            {
            // NOTE: Presence of subRange tag is all we care about, values aren't used as they will be computed from geometry...
            if (checkedSubGraphics)
                break;

            // NOTE: Worth including range whenever there are multiple primitives, not just when multiple primitives follow the subRange tag...
            if (n > 2 && i+1 < n)
                {
                size_t iGeom = 0;

                for (int j = 0; j < n; j++)
                    {
                    auto rangeEntry = input[j];

                    // Check for known non-geometry tags...for counting purposes, a GeometryPart is considered geometry (i.e. stream includes mix of part instances and primitives)...
                    if (!rangeEntry.isObject() || rangeEntry.isMember("appearance") || rangeEntry.isMember("styleMod") || rangeEntry.isMember("fill") || rangeEntry.isMember("pattern") || rangeEntry.isMember("material") || rangeEntry.isMember("subRange"))
                        continue;

                    if (++iGeom > 1)
                        break;
                    }

                if (iGeom > 1)
                    SetAppendAsSubGraphics(); // This will do nothing if a GeometryPart is being created, or a GeometryPart instance is being inserted...
                }

            checkedSubGraphics = true;
            }
        else if (entry.isMember("textString"))
            {
            auto textString = entry["textString"];

            if (textString["font"].isNull() || textString["height"].isNull() || textString["text"].isNull())
                return false; // A font, height, and text are required...

            double  height = textString["height"].asDouble();
            double  width = height;

            if (0.0 == height)
                return false; // Invalid height...

            if (!textString["widthFactor"].isNull())
                {
                double widthFactor = textString["widthFactor"].asDouble();

                if (0.0 != widthFactor)
                    width = height * widthFactor;
                }

            TextStringStylePtr style = TextStringStyle::Create();
            style->SetFont(FontId((uint64_t) textString["font"].asInt()));
            style->SetSize(width, height);

            if (!textString["bold"].isNull())
                style->SetIsBold(textString["bold"].asBool());

            if (!textString["italic"].isNull())
                style->SetIsItalic(textString["italic"].asBool());

            if (!textString["underline"].isNull())
                style->SetIsUnderlined(textString["underline"].asBool());

            TextStringPtr text = TextString::Create(GetDgnDb());

            text->SetStyle(*style);
            text->SetText(textString["text"].asString().c_str());

            if (!textString["origin"].isNull())
                {
                DPoint3d origin;

                BeJsGeomUtils::DPoint3dFromJson(origin, textString["origin"]);
                text->SetOrigin(origin);
                }

            if (!textString["rotation"].isNull())
                {
                YawPitchRollAngles angles = BeJsGeomUtils::YawPitchRollFromJson(textString["rotation"]);
                RotMatrix rMatrix = angles.ToRotMatrix();

                text->SetOrientation(rMatrix);
                }

            if (!Append(params))
                return false;

            if (!Append(*text))
                return false;
            }
        else if (entry.isMember("brep"))
            {
            auto brep = entry["brep"];
            IBRepEntityPtr entity = PlatformLib::Host::BRepGeometryAdmin::BodyFromJson(brep);

            if (!entity.IsValid())
                return false;

            if (!Append(params))
                return false;

            if (!Append(*entity))
                return false;
            }
        else if (entry.isMember("image"))
            {
            auto img = ImageGraphic::FromJson(entry["image"]);
            if (img.IsNull() || !Append(params) || !Append(*img))
                return false;
            }
        else
            {
            bvector<IGeometryPtr> geometry;

            if (!IModelJson::TryIModelJsonValueToGeometry(entry, geometry))
                continue; // Should a tag we don't recognize be considered an error? Should at least log an error...

            if (1 != geometry.size())
                return false; // Should only ever be a single entry...

            if (!Append(params))
                return false;

            if (!Append(*geometry.at(0)))
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::UpdateFromJson(DgnGeometryPartR part, BeJsConst input)
    {
    if (!input.isArray())
        return false;

    GeometryBuilderPtr builder = CreateGeometryPart(part.GetDgnDb(), true); // NEEDSWORK...supply 2d/3d in opts?

    if (!builder.IsValid())
        return false;

    if (!builder->FromJson(input))
        return false;

    return (SUCCESS == builder->Finish(part));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::UpdateFromJson(GeometrySourceR source, BeJsConst input)
    {
    if (!input.isArray())
        return false;

    GeometryBuilderPtr builder = Create(source);

    if (!builder.IsValid())
        return false;

    if (!builder->FromJson(input))
        return false;

    return (SUCCESS == builder->Finish(source));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::UpdateFromJsonEntryArray(GeometrySourceR source, BeJsConst input)
    {
    if (!input.isArray())
        return false;

    GeometryBuilderPtr builder = Create(source);

    if (!builder.IsValid())
        return false;

    Render::GeometryParams geomParams = builder->GetGeometryParams();
    GeometryStreamIO::Reader reader(builder->GetDgnDb());
    ByteStream data;

    for (uint32_t index = 0, nEntries = input.size(); index < nEntries; ++index)
        {
        auto entry = input[index];
        if (!entry.isObject())
            continue;

        if (!entry.isMember("opcode") || !entry.isMember("data"))
            continue;

        auto opCode = static_cast<GeometryStreamIO::OpCode>(entry["opcode"].asUInt());
        entry["data"].GetBinary(data);
        if (!data.HasData())
            continue;

        GeometryStreamIO::Operation egOp(opCode, data.GetSize(), data.GetData());

        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                break; // Ignore if present...

            case GeometryStreamIO::OpCode::BasicSymbology:
            case GeometryStreamIO::OpCode::LineStyleModifiers:
            case GeometryStreamIO::OpCode::AreaFill:
            case GeometryStreamIO::OpCode::Pattern:
            case GeometryStreamIO::OpCode::Material:
                {
                if (!reader.Get(egOp, geomParams))
                    return false;

                if (!builder->Append(geomParams))
                    return false;
                break;
                }

            case GeometryStreamIO::OpCode::SubGraphicRange:
                {
                // Good enough...only being used for dynamics currently...
                if (nEntries > 1)
                    builder->SetAppendAsSubGraphics();
                break;
                }

            case GeometryStreamIO::OpCode::GeometryPartInstance:
                {
                DgnGeometryPartId geomPartId;
                Transform         geomToSource;

                if (!reader.Get(egOp, geomPartId, geomToSource))
                    return false;

                if (!builder->Append(geomPartId, geomToSource))
                    return false;
                break;
                }

            default:
                {
                if (!egOp.IsGeometryOp())
                    return false;

                GeometricPrimitivePtr geom;
                if (!reader.Get(egOp, geom, true))
                    return false;

                if (!builder->Append(*geom))
                    return false;
                break;
                }
            }
        }

    return (SUCCESS == builder->Finish(source));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageGraphic::ToJson(BeJsValue json) const
    {
    auto cornersJson = json["corners"];
    for (auto i = 0; i < 4; i++)
        BeJsGeomUtils::DPoint3dToJson(cornersJson[i], GetCorners().m_pts[i]);

    json["textureId"] = GetTextureId().ToHexStr();
    json["hasBorder"] = HasBorder();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ImageGraphicPtr ImageGraphic::FromJson(BeJsConst json)
    {
    auto const& cornersJson = json["corners"];
    if (!cornersJson.isArray() || 4 != cornersJson.size())
        return nullptr;

    DgnTextureId textureId;
    textureId.FromJson(json["textureId"]);
    if (!textureId.IsValid())
        return nullptr;

    Corners corners;
    for (auto i = 0; i < 4; i++)
        BeJsGeomUtils::DPoint3dFromJson(corners.m_pts[i], cornersJson[i]);

    return ImageGraphic::Create(corners, textureId, json["hasBorder"].asBool());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr ImageGraphic::ToCurvePrimitive() const
    {
    auto imgPts = GetCorners().m_pts;
    DPoint3d points[5] = { imgPts[0], imgPts[1], imgPts[2], imgPts[3], imgPts[0] };
    return ICurvePrimitive::CreateLineString(points, 5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr ImageGraphic::ToCurveVector(CurveVector::BoundaryType boundary) const
    {
    return CurveVector::Create(boundary, ToCurvePrimitive());
    }
