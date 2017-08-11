/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementGeometry.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <GeomSerialization/GeomLibsFlatBufferApi.h>
#include <DgnPlatformInternal/DgnCore/ElementGraphics.fb.h>
#include <DgnPlatformInternal/DgnCore/TextStringPersistence.h>
#include "DgnPlatform/Annotations/TextAnnotationDraw.h"
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

using namespace flatbuffers;

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageGraphic::CreateTexture(ViewContextR context)
    {
    if (!m_texture.IsValid())
        {
        DgnViewportP vp = context.GetViewport(); // NEEDSWORK: For mesh tiles, ask context for render target...

        m_texture = (nullptr != vp ? vp->GetRenderTarget()->CreateTexture(m_image) : nullptr);
        }

    return m_texture.IsValid();
    };

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageGraphic::AddToGraphic(Render::GraphicBuilderR graphic) const
    {
    if (m_texture.IsValid())
        graphic.AddTile(*m_texture, m_corners);

    if (!m_drawBorder && m_texture.IsValid()) // Always show border when texture isn't available...
        return;

    DPoint3d pts[5];

    pts[0] = m_corners.m_pts[0];
    pts[1] = m_corners.m_pts[1];
    pts[2] = m_corners.m_pts[3];
    pts[3] = m_corners.m_pts[2];
    pts[4] = pts[0];

    if (m_texture.IsValid())
        graphic.AddLineString(5, pts);
    else
        graphic.AddShape(5, pts, true); // Draw filled shape for pick...
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageGraphic::AddToGraphic2d(Render::GraphicBuilderR graphic, double displayPriority) const
    {
    GraphicBuilder::TileCorners corners = m_corners;

    corners.m_pts[0].z = corners.m_pts[1].z = corners.m_pts[2].z = corners.m_pts[3].z = displayPriority;

    if (m_texture.IsValid())
        graphic.AddTile(*m_texture, corners);

    if (!m_drawBorder && m_texture.IsValid()) // Always show border when texture isn't available...
        return;

    DPoint2d pts[5];

    pts[0].Init(corners.m_pts[0]);
    pts[1].Init(corners.m_pts[1]);
    pts[2].Init(corners.m_pts[3]);
    pts[3].Init(corners.m_pts[2]);
    pts[4] = pts[0];

    if (m_texture.IsValid())
        graphic.AddLineString2d(5, pts, displayPriority);
    else
        graphic.AddShape2d(5, pts, true, displayPriority); // Draw filled shape for pick...
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
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
            GraphicBuilder::TileCorners const& corners = GetAsImage()->GetTileCorners();
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
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricPrimitive::GetLocalRange(DRange3dR localRange, TransformR localToWorld) const
    {
    if (!GetLocalCoordinateFrame(localToWorld))
        return false;

    if (localToWorld.IsIdentity())
        return GetRange(localRange);

#if defined (BENTLEYCONFIG_PARASOLID)
    GeometricPrimitivePtr clone;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (GeometryType::BRepEntity == GetGeometryType())
        clone = new GeometricPrimitive(PSolidUtil::InstanceEntity(*GetAsIBRepEntity()));
    else
        clone = Clone();
#else
    GeometricPrimitivePtr clone = Clone();
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
    Transform originWithExtentVectors, centroidalLocalToWorld, centroidalWorldToLocal;

    // NOTE: MSBsplineSurface::GetPoleRange can result in a very large range...but using IPolyfaceConstruction is too slow...
    if (geom.TightPrincipalExtents(originWithExtentVectors, centroidalLocalToWorld, centroidalWorldToLocal, range))
        {
        centroidalLocalToWorld.Multiply(range, range);

        if (nullptr != transform)
            transform->Multiply(range, range);

        return true;
        }

    if (nullptr != transform)
        geom.GetPoleRange(range, *transform);
    else
        geom.GetPoleRange(range);

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
* @bsimethod                                                    Jeff.Marker     05/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(TextStringCR text, DRange3dR range, TransformCP transform)
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
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getRange(ImageGraphicCR geom, DRange3dR range, TransformCP transform)
    {
    range = geom.GetRange();

    if (nullptr != transform)
        transform->Multiply(range, range);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
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

        case GeometryType::TextString:
            {
            TextStringCR geom = *GetAsTextString();

            return getRange(geom, range, transform);
            }

        case GeometryType::Image:
            {
            ImageGraphicCR geom = *GetAsImage();

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
            {
            ImageGraphicR geom = *GetAsImage();

            geom.ApplyTransform(transform);
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

#if defined (BENTLEYCONFIG_PARASOLID)
        case GeometryType::BRepEntity:
            {
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

        case GeometryType::TextString: // <- Don't currently need to compare TextString...
        case GeometryType::Image: // <- Don't currently need to compare Images...
        default:
            return false;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/15
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
            {
            ImageGraphicPtr geom = GetAsImage()->Clone();

            return new GeometricPrimitive(geom);
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
            {
            ImageGraphicCR image = *GetAsImage();

            image.AddToGraphic(graphic);
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
* @bsimethod                                                    Brien.Bastings  07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricPrimitive::IsSolid() const
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
* @bsimethod                                                    Brien.Bastings  07/16
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
GeometricPrimitive::GeometricPrimitive(ICurvePrimitivePtr const& source) {m_type = GeometryType::CurvePrimitive; m_data = source;}
GeometricPrimitive::GeometricPrimitive(CurveVectorPtr const& source) {m_type = GeometryType::CurveVector; m_data = source;}
GeometricPrimitive::GeometricPrimitive(ISolidPrimitivePtr const& source) {m_type = GeometryType::SolidPrimitive; m_data = source;}
GeometricPrimitive::GeometricPrimitive(MSBsplineSurfacePtr const& source) {m_type = GeometryType::BsplineSurface; m_data = source;}
GeometricPrimitive::GeometricPrimitive(PolyfaceHeaderPtr const& source) {m_type = GeometryType::Polyface; m_data = source;}
GeometricPrimitive::GeometricPrimitive(IBRepEntityPtr const& source) {m_type = GeometryType::BRepEntity; m_data = source;}
GeometricPrimitive::GeometricPrimitive(TextStringPtr const& source) {m_type = GeometryType::TextString; m_data = source;}
GeometricPrimitive::GeometricPrimitive(ImageGraphicPtr const& source) {m_type = GeometryType::Image; m_data = source;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GeometricPrimitive::Create(ICurvePrimitivePtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(CurveVectorPtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(ISolidPrimitivePtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(MSBsplineSurfacePtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(PolyfaceHeaderPtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(IBRepEntityPtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(TextStringPtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}
GeometricPrimitivePtr GeometricPrimitive::Create(ImageGraphicPtr const& source) {return (source.IsValid() ? new GeometricPrimitive(source) : nullptr);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GeometricPrimitive::Create(ICurvePrimitiveCR source) {ICurvePrimitivePtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(CurveVectorCR source) {CurveVectorPtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(ISolidPrimitiveCR source) {ISolidPrimitivePtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(MSBsplineSurfaceCR source) {MSBsplineSurfacePtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(PolyfaceQueryCR source) {PolyfaceHeaderPtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(IBRepEntityCR source) {IBRepEntityPtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(TextStringCR source) {TextStringPtr clone = source.Clone(); return Create(clone);}
GeometricPrimitivePtr GeometricPrimitive::Create(ImageGraphicCR source) {ImageGraphicPtr clone = source.Clone(); return Create(clone);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Shaun.Sewall    02/17
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GeometricPrimitive::Create(DEllipse3dCR source) {ICurvePrimitivePtr curve = ICurvePrimitive::CreateArc(source); return curve.IsValid() ? Create(curve) : nullptr;}
GeometricPrimitivePtr GeometricPrimitive::Create(DgnBoxDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnBox(source); return solid.IsValid() ? Create(solid) : nullptr;}
GeometricPrimitivePtr GeometricPrimitive::Create(DgnConeDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnCone(source); return solid.IsValid() ? Create(solid) : nullptr;}
GeometricPrimitivePtr GeometricPrimitive::Create(DgnSphereDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnSphere(source); return solid.IsValid() ? Create(solid) : nullptr;}
GeometricPrimitivePtr GeometricPrimitive::Create(DgnTorusPipeDetailCR source) {ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnTorusPipe(source); return solid.IsValid() ? Create(solid) : nullptr;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitive::GeometryType GeometricPrimitive::GetGeometryType() const {return m_type;}
ICurvePrimitivePtr GeometricPrimitive::GetAsICurvePrimitive() const {return (GeometryType::CurvePrimitive == m_type ? static_cast <ICurvePrimitiveP> (m_data.get()) : nullptr);}
CurveVectorPtr GeometricPrimitive::GetAsCurveVector() const {return (GeometryType::CurveVector == m_type ? static_cast <CurveVectorP> (m_data.get()) : nullptr);}
ISolidPrimitivePtr GeometricPrimitive::GetAsISolidPrimitive() const {return (GeometryType::SolidPrimitive == m_type ? static_cast <ISolidPrimitiveP> (m_data.get()) : nullptr);}
MSBsplineSurfacePtr GeometricPrimitive::GetAsMSBsplineSurface() const {return (GeometryType::BsplineSurface == m_type ? static_cast <RefCountedMSBsplineSurface*> (m_data.get()) : nullptr);}
PolyfaceHeaderPtr GeometricPrimitive::GetAsPolyfaceHeader() const {return (GeometryType::Polyface == m_type ? static_cast <PolyfaceHeaderP> (m_data.get()) : nullptr);}
IBRepEntityPtr GeometricPrimitive::GetAsIBRepEntity() const {return (GeometryType::BRepEntity == m_type ? static_cast <IBRepEntityP> (m_data.get()) : nullptr);}
TextStringPtr GeometricPrimitive::GetAsTextString() const {return (GeometryType::TextString == m_type ? static_cast <TextStringP> (m_data.get()) : nullptr);}
ImageGraphicPtr GeometricPrimitive::GetAsImage() const {return (GeometryType::Image == m_type ? static_cast <ImageGraphicP> (m_data.get()) : nullptr);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/15
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
#if defined (BENTLEYCONFIG_PARASOLID)
        case OpCode::ParasolidBRep:
#else
        case OpCode::BRepPolyface:
        case OpCode::BRepCurveVector:
#endif
        case OpCode::TextString:
        case OpCode::Image:
            return true;

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2014
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
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(DPoint2dCP pts, size_t nPts, int8_t boundary)
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
void GeometryStreamIO::Writer::Append(DPoint3dCP pts, size_t nPts, int8_t boundary)
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
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(DRange3dCR range)
    {
    FlatBufferBuilder fbb;

    auto coords = fbb.CreateVectorOfStructs((FB::DPoint3d*) &range.low, 6);

    FB::PointPrimitiveBuilder builder(fbb);

    builder.add_coords(coords);

    auto mloc = builder.Finish();

    fbb.Finish(mloc);
    Append(Operation(OpCode::SubGraphicRange, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(DEllipse3dCR arc, int8_t boundary)
    {
    FlatBufferBuilder fbb;

    auto mloc = FB::CreateArcPrimitive(fbb, (FB::DPoint3d*) &arc.center, (FB::DVec3d*) &arc.vector0, (FB::DVec3d*) &arc.vector90, arc.start, arc.sweep, (FB::BoundaryType) boundary);

    fbb.Finish(mloc);
    Append(Operation(OpCode::ArcPrimitive, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool hasDisconnectPoint(DPoint3dCP pts, size_t nPts)
    {
    for (size_t iPt = 0; iPt < nPts; ++iPt)
        {
        if (pts[iPt].IsDisconnect())
            {
            BeAssert(false); // STOP USING THIS ABOMINATION! Create a BOUNDARY_TYPE_None CurveVector with disjoint pieces... 
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2017
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
* @bsimethod                                                    Brien.Bastings  03/2017
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
* @bsimethod                                                    Brien.Bastings  12/2014
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
* @bsimethod                                                    Brien.Bastings  12/2014
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
* @bsimethod                                                    Brien.Bastings  12/2014
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
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(CurveVectorCR curves, OpCode opCode)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes(curves, buffer);

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
void GeometryStreamIO::Writer::Append(ICurvePrimitiveCR curvePrimitive)
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
void GeometryStreamIO::Writer::Append(PolyfaceQueryCR meshData, OpCode opCode)
    {
    bvector<Byte> buffer;

    BentleyGeometryFlatBuffer::GeometryToBytes(meshData, buffer);

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
* @bsimethod                                                    Brien.Bastings  12/2014
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
* @bsimethod                                                    Brien.Bastings  06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(IBRepEntityCR entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    size_t      bufferSize = 0;
    uint8_t*    buffer = nullptr;

    if (SUCCESS != PSolidUtil::SaveEntityToMemory(&buffer, bufferSize, entity))
        {
        BeAssert(false);
        return;
        }

    IFaceMaterialAttachmentsCP attachments = entity.GetFaceMaterialAttachments();
    bvector<FB::FaceSymbology> fbSymbVec;
    bvector<FB::FaceSymbologyIndex> fbSymbIndexVec;

    if (nullptr != attachments)
        {
        T_FaceAttachmentsVec const& faceAttachmentsVec = attachments->_GetFaceAttachmentsVec();

        for (FaceAttachment attachment : faceAttachmentsVec)
            {
            FB::DPoint2d    uv(0.0, 0.0); // NEEDSWORK_WIP_MATERIAL - Add geometry specific material mappings to GeometryParams/GraphicParams...
            GeometryParams  faceParams, baseParamsIgnored;

            attachment.ToGeometryParams(faceParams, baseParamsIgnored);

            bool useColor = !faceParams.IsLineColorFromSubCategoryAppearance();
            bool useMaterial = !faceParams.IsMaterialFromSubCategoryAppearance();

            FB::FaceSymbology  fbSymb(useColor, useMaterial,
                                      useColor ? faceParams.GetLineColor().GetValue() : 0,
                                      useMaterial ? faceParams.GetMaterialId().GetValueUnchecked() : 0,
                                      useColor ? faceParams.GetTransparency() : 0, uv);

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
    Transform entityTransform = entity.GetEntityTransform();

    builder.add_entityTransform((FB::Transform*) &entityTransform);
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

    // NOTE: Append redundant representation for platforms where we don't yet have support for Parasolid...
    switch (entity.GetEntityType())
        {
        case IBRepEntity::EntityType::Wire:
            {
            // Save wire body as CurveVector...
            CurveVectorPtr wireGeom = PSolidGeom::WireBodyToCurveVector(entity);

            if (wireGeom.IsValid())
                Append(*wireGeom, OpCode::BRepCurveVector);

            return;
            }

        case IBRepEntity::EntityType::Sheet:
            {
            // Save sheet body that is a single planar face as CurveVector...
            CurveVectorPtr faceGeom = PSolidGeom::PlanarSheetBodyToCurveVector(entity);

            if (faceGeom.IsValid())
                {
                Append(*faceGeom, OpCode::BRepCurveVector);
                return;
                }

            // Fall through...
            }

        case IBRepEntity::EntityType::Solid:
            {
            IFacetOptionsPtr  facetOpt = IFacetOptions::CreateForCurves();

            facetOpt->SetAngleTolerance(0.2); // NOTE: This is the value XGraphics "optimize" used...
            facetOpt->SetNormalsRequired(true);
            facetOpt->SetParamsRequired(true);

            if (nullptr != attachments)
                {
                bvector<PolyfaceHeaderPtr> polyfaces;
                bvector<FaceAttachment> params;

                BRepUtil::FacetEntity(entity, polyfaces, params, *facetOpt);

                for (size_t i = 0; i < polyfaces.size(); i++)
                    {
                    if (0 == polyfaces[i]->GetPointCount())
                        continue;

                    GeometryParams  faceParams, baseParamsIgnored;

                    params[i].ToGeometryParams(faceParams, baseParamsIgnored);
                    Append(faceParams, true, true); // We don't support allowing sub-category to vary by FaceAttachment...and we didn't initialize it...
                    polyfaces[i]->NormalizeParameters(); // Normalize uv parameters or materials won't have correct scale...
                    Append(*polyfaces[i], OpCode::BRepPolyface);
                    }
                }
            else
                {
                PolyfaceHeaderPtr polyface = BRepUtil::FacetEntity(entity, *facetOpt);

                if (polyface.IsValid())
                    {
                    polyface->NormalizeParameters(); // Normalize uv parameters or materials won't have correct scale...
                    Append(*polyface, OpCode::BRepPolyface);
                    }
                }
            break;
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
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
* @bsimethod                                                    Brien.Bastings  12/2014
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
                                                      (FB::DPoint3d*)&lsParams->normal, angles.GetYaw().Degrees(), angles.GetPitch().Degrees(), angles.GetRoll().Degrees());
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

            auto mloc = FB::CreateAreaFill(fbb, (FB::FillDisplay) elParams.GetFillDisplay(),
                                           0, 0, 0, elParams.GetFillTransparency(),
                                           (FB::GradientMode) gradient.GetMode(), gradient.GetFlags(),
                                           gradient.GetAngle(), gradient.GetTint(), gradient.GetShift(),
                                           colors, values);
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

        builder.add_origin((FB::DPoint3d*) &pattern->GetOrigin());

        if (!pattern->GetOrientation().IsIdentity())
            builder.add_rotation((FB::RotMatrix*) &pattern->GetOrientation());

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

    if (useMaterial)
        {
        FlatBufferBuilder fbb;

        auto mloc = FB::CreateMaterial(fbb, useMaterial, useMaterial && elParams.GetMaterialId().IsValid() ? elParams.GetMaterialId().GetValueUnchecked() : 0, nullptr, nullptr, 0.0, 0.0, 0.0);
        fbb.Finish(mloc);

        Append(Operation(OpCode::Material, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
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
* @bsimethod                                                    Brien.Bastings  02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(ImageGraphicCR source)
    {
    if (!source.GetImage().IsValid())
        return;

    FlatBufferBuilder fbb;

    auto byteData = fbb.CreateVector(source.GetImage().GetByteStream().GetData(), source.GetImage().GetByteStream().GetSize());

    FB::ImageBuilder builder(fbb);
    GraphicBuilder::TileCorners const& corners = source.GetTileCorners();

    builder.add_tileCorner0((FB::DPoint3d*) &corners.m_pts[0]);
    builder.add_tileCorner1((FB::DPoint3d*) &corners.m_pts[1]);
    builder.add_tileCorner2((FB::DPoint3d*) &corners.m_pts[2]);
    builder.add_tileCorner3((FB::DPoint3d*) &corners.m_pts[3]);
    builder.add_drawBorder(source.GetDrawBorder());
    builder.add_useFillTint(source.GetUseFillTint());
    builder.add_width(source.GetImage().GetWidth());
    builder.add_height(source.GetImage().GetHeight());
    builder.add_format((uint32_t) source.GetImage().GetFormat());
    builder.add_byteData(byteData);

    auto mloc = builder.Finish();

    fbb.Finish(mloc);
    Append(Operation(OpCode::Image, (uint32_t) fbb.GetSize(), fbb.GetBufferPointer()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Writer::Append(TextStringCR text)
    {
    bvector<Byte> data;
    if (SUCCESS != TextStringPersistence::EncodeAsFlatBuf(data, text, m_db, TextStringPersistence::FlatBufEncodeOptions::IncludeGlyphLayoutData))
        return;

    Append(Operation(OpCode::TextString, (uint32_t)data.size(), &data[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
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
* @bsimethod                                                    Brien.Bastings  12/2014
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
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, DRange3dR range) const
    {
    if (OpCode::SubGraphicRange != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::PointPrimitive>(egOp.m_data);

    if (6 != ppfb->coords()->Length())
        return false;

    memcpy(&range, (DPoint3dCP) ppfb->coords()->Data(), sizeof(range));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
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
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, ICurvePrimitivePtr& curve) const
    {
    if (OpCode::CurvePrimitive != egOp.m_opCode)
        return false;

    curve = BentleyGeometryFlatBuffer::BytesToCurvePrimitive(egOp.m_data);

    return curve.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, CurveVectorPtr& curves) const
    {
    if (OpCode::CurveVector != egOp.m_opCode)
        return false;

    curves = BentleyGeometryFlatBuffer::BytesToCurveVector(egOp.m_data);

    return curves.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, PolyfaceQueryCarrier& meshData) const
    {
    if (OpCode::Polyface != egOp.m_opCode)
        return false;

    return BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier(egOp.m_data, meshData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, ISolidPrimitivePtr& solid) const
    {
    if (OpCode::SolidPrimitive != egOp.m_opCode)
        return false;

    solid = BentleyGeometryFlatBuffer::BytesToSolidPrimitive(egOp.m_data);

    return solid.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, MSBsplineSurfacePtr& surface) const
    {
    if (OpCode::BsplineSurface != egOp.m_opCode)
        return false;

    surface = BentleyGeometryFlatBuffer::BytesToMSBsplineSurface(egOp.m_data);

    return surface.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, IBRepEntityPtr& entity) const
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    if (OpCode::ParasolidBRep != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::BRepData>(egOp.m_data);

    // NOTE: It's possible to check ppfb->brepType() to avoid calling restore in order to check type...
    if (SUCCESS != PSolidUtil::RestoreEntityFromMemory(entity, ppfb->entityData()->Data(), ppfb->entityData()->Length(), *((TransformCP) ppfb->entityTransform())))
        return false;

    if (!ppfb->has_symbology() || !ppfb->has_symbologyIndex())
        return true;

    for (size_t iSymb=0; iSymb < ppfb->symbology()->Length(); iSymb++)
        {
        FB::FaceSymbology const* fbSymb = ((FB::FaceSymbology const*) ppfb->symbology()->Data())+iSymb;
        GeometryParams faceParams;

        if (fbSymb->useColor())
            {
            faceParams.SetLineColor(ColorDef(fbSymb->color()));
            faceParams.SetTransparency(fbSymb->transparency());
            }

        if (fbSymb->useMaterial())
            {
            faceParams.SetMaterialId(RenderMaterialId((uint64_t)fbSymb->materialId()));
            // NEEDSWORK_WIP_MATERIAL...uv???
            }

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
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
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
* @bsimethod                                                    Brien.Bastings  12/2014
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
                    uint32_t* colors = (uint32_t*) ppfb->colors()->Data();
                    bvector<ColorDef> keyColors;

                    for (uint32_t iColor=0; iColor < nColors; ++iColor)
                        keyColors.push_back(ColorDef(colors[iColor]));

                    gradientPtr->SetKeys((uint32_t) keyColors.size(), &keyColors.front(), (double*) ppfb->values()->Data());
                    elParams.SetGradient(gradientPtr.get());
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
                    line.m_nDashes = fbDefLine.dashes()->Length();

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
bool GeometryStreamIO::Reader::Get(Operation const& egOp, TextStringR text) const
    {
    if (OpCode::TextString != egOp.m_opCode)
        return false;

    return (SUCCESS == TextStringPersistence::DecodeFromFlatBuf(text, egOp.m_data, egOp.m_dataSize, m_db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, ImageGraphicPtr& out) const
    {
    if (OpCode::Image != egOp.m_opCode)
        return false;

    auto ppfb = flatbuffers::GetRoot<FB::Image>(egOp.m_data);

    ByteStream byteData(ppfb->byteData()->Data(), ppfb->byteData()->Length());
    Render::Image image(ppfb->width(), ppfb->height(), std::move(byteData), (Render::Image::Format) ppfb->format());
    GraphicBuilder::TileCorners corners;

    corners.m_pts[0] = (nullptr == ppfb->tileCorner0() ? DPoint3d::FromZero() : *((DPoint3dCP) ppfb->tileCorner0()));
    corners.m_pts[1] = (nullptr == ppfb->tileCorner1() ? DPoint3d::FromZero() : *((DPoint3dCP) ppfb->tileCorner1()));
    corners.m_pts[2] = (nullptr == ppfb->tileCorner2() ? DPoint3d::FromZero() : *((DPoint3dCP) ppfb->tileCorner2()));
    corners.m_pts[3] = (nullptr == ppfb->tileCorner3() ? DPoint3d::FromZero() : *((DPoint3dCP) ppfb->tileCorner3()));

    out = ImageGraphic::Create(std::move(image), corners, ppfb->drawBorder(), ppfb->useFillTint());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStreamIO::Reader::Get(Operation const& egOp, GeometricPrimitivePtr& elemGeom) const
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

            if (!Get(egOp, curvePtr))
                break;

            elemGeom = GeometricPrimitive::Create(curvePtr);
            return true;
            }

        case GeometryStreamIO::OpCode::CurveVector:
            {
            CurveVectorPtr curvePtr;

            if (!Get(egOp, curvePtr))
                break;

            elemGeom = GeometricPrimitive::Create(curvePtr);
            return true;
            }

        case GeometryStreamIO::OpCode::Polyface:
            {
            PolyfaceQueryCarrier meshData(0, false, 0, 0, nullptr, nullptr);

            if (!Get(egOp, meshData))
                break;

            elemGeom = GeometricPrimitive::Create(meshData);
            return true;
            }

        case GeometryStreamIO::OpCode::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPtr;

            if (!Get(egOp, solidPtr))
                break;

            elemGeom = GeometricPrimitive::Create(solidPtr);
            return true;
            }

        case GeometryStreamIO::OpCode::BsplineSurface:
            {
            MSBsplineSurfacePtr surfacePtr;

            if (!Get(egOp, surfacePtr))
                break;

            elemGeom = GeometricPrimitive::Create(surfacePtr);
            return true;
            }

#if defined (BENTLEYCONFIG_PARASOLID) 
        case GeometryStreamIO::OpCode::ParasolidBRep:
            {
            IBRepEntityPtr entityPtr;

            if (!Get(egOp, entityPtr))
                break;

            elemGeom = GeometricPrimitive::Create(entityPtr);
            return true;
            }
#else
        case GeometryStreamIO::OpCode::BRepPolyface:
            {
            PolyfaceQueryCarrier meshData(0, false, 0, 0, nullptr, nullptr);

            if (!BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier(egOp.m_data, meshData))
                break;

            elemGeom = GeometricPrimitive::Create(meshData);
            return true;
            }

        case GeometryStreamIO::OpCode::BRepCurveVector:
            {
            CurveVectorPtr curvePtr = BentleyGeometryFlatBuffer::BytesToCurveVector(egOp.m_data);

            if (!curvePtr.IsValid())
                break;

            elemGeom = GeometricPrimitive::Create(curvePtr);
            return true;
            }
#endif

        case GeometryStreamIO::OpCode::TextString:
            {
            TextStringPtr text = TextString::Create();
            if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(*text, egOp.m_data, egOp.m_dataSize, m_db))
                break;

            elemGeom = GeometricPrimitive::Create(text);
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Iterator::ToNext()
    {
    if (m_dataOffset >= m_dataSize)
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
* @bsimethod                                                    Sam.Wilson      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStreamIO::Import(GeometryStreamR dest, GeometryStreamCR source, DgnImportContext& importer)
    {
    if (!source.HasGeometry())
        return DgnDbStatus::Success; // otherwise we end up writing a header for an otherwise empty stream...

    Writer writer(importer.GetDestinationDb());
    Collection collection(source.GetData(), source.GetSize());

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
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
                auto mloc = FB::CreateMaterial(remappedfbb, fbSymb->useMaterial(), remappedMaterialId.GetValueUnchecked(), fbSymb->origin(), fbSymb->size(), fbSymb->yaw(), fbSymb->pitch(), fbSymb->roll());
                remappedfbb.Finish(mloc);
                writer.Append(Operation(OpCode::Material, (uint32_t) remappedfbb.GetSize(), remappedfbb.GetBufferPointer()));
                break;
                }

            case GeometryStreamIO::OpCode::ParasolidBRep:
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
                auto remappedFaceSymbIndex = remappedfbb.CreateVectorOfStructs((FB::FaceSymbologyIndex const*) ppfb->symbologyIndex()->Data(), ppfb->symbologyIndex()->Length());
                auto mloc = FB::CreateBRepData(remappedfbb, ppfb->entityTransform(), ppfb->brepType(), remappedEntityData, remappedFaceSymb, remappedFaceSymbIndex);
                remappedfbb.Finish(mloc);
                writer.Append(Operation(OpCode::ParasolidBRep, (uint32_t) remappedfbb.GetSize(), remappedfbb.GetBufferPointer()));
                break;
                }

            case GeometryStreamIO::OpCode::TextString:
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
* @bsimethod                                                    Brien.Bastings  12/2015
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
            geomType.assign("ImageGraphic");
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
* @bsimethod                                                    Brien.Bastings  10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Debug(IDebugOutput& output, GeometryStreamCR stream, DgnDbR db, bool isPart)
    {
    Collection  collection(stream.GetData(), stream.GetSize());
    Reader      reader(db);

    IdSet<DgnGeometryPartId> parts;

    for (auto const& egOp : collection)
        {
        switch (egOp.m_opCode)
            {
            case GeometryStreamIO::OpCode::Header:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::Header\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::SubGraphicRange:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::SubGraphicRange\n").c_str());
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

                output._DoOutputLine(Utf8PrintfString("  ").c_str());

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

                output._DoOutputLine(Utf8PrintfString("\n").c_str());
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

            case GeometryStreamIO::OpCode::PointPrimitive:
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

            case GeometryStreamIO::OpCode::PointPrimitive2d:
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

            case GeometryStreamIO::OpCode::ArcPrimitive:
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

            case GeometryStreamIO::OpCode::CurveVector:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::CurveVector\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::Polyface:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::Polyface\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::CurvePrimitive:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::CurvePrimitive\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::SolidPrimitive:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::SolidPrimitive\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::BsplineSurface:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::BsplineSurface\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::ParasolidBRep:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::ParasolidBRep\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::BRepPolyface:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::BRepPolyface\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::BRepCurveVector:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::BRepCurveVector\n").c_str());
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

                    output._DoOutputLine(Utf8PrintfString("\n").c_str());
                    break;
                    }

                if (!(ppfb->has_color() || ppfb->has_useColor() || ppfb->has_backgroundFill() || ppfb->has_transparency()))
                    break;

                output._DoOutputLine(Utf8PrintfString("  ").c_str());

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
                    output._DoOutputLine(Utf8PrintfString("Fill: Single Color ").c_str());
                    }

                if (ppfb->has_transparency())
                    output._DoOutputLine(Utf8PrintfString("Transparency: %lf ", ppfb->transparency()).c_str());

                output._DoOutputLine(Utf8PrintfString("\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::Pattern:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::Pattern\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::Material:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::Material\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::LineStyleModifiers:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::LineStyleModifiers\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::TextString:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::TextString\n").c_str());
                break;
                }

            case GeometryStreamIO::OpCode::Image:
                {
                output._DoOutputLine(Utf8PrintfString("OpCode::ImageGraphic\n").c_str());
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
        GeometryCollection collection(stream, db);

        output._DoOutputLine(Utf8PrintfString("\n--- GeometryStream Entry Ids ---\n\n").c_str());

        for (auto iter : collection)
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
                GeometricPrimitivePtr partGeom = partIter.GetGeometryPtr();

                if (!partGeom.IsValid())
                    continue;

                debugGeomId(output, *partGeom, partIter.GetGeometryStreamEntryId());
                }
            }

        output._DoOutputLine("\n");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2015
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

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/2016
+===============+===============+===============+===============+===============+======*/
struct BRepCache : DgnElement::AppData
{
static DgnElement::AppData::Key const& GetKey() {static DgnElement::AppData::Key s_key; return s_key;}
typedef bmap<uint16_t, IBRepEntityPtr> IndexedGeomMap;
IndexedGeomMap m_map;

virtual DropMe _OnInserted(DgnElementCR el){return DropMe::Yes;}
virtual DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal) {return DropMe::Yes;}
virtual DropMe _OnAppliedUpdate(DgnElementCR original, DgnElementCR modified) {return DropMe::Yes;}
virtual DropMe _OnDeleted(DgnElementCR el) {return DropMe::Yes;}

static BRepCache* Get(DgnElementCR elem, bool addIfNotFound)
    {
    BRepCache* cache = dynamic_cast<BRepCache*>(elem.FindAppData(GetKey()));

    if (nullptr == cache && addIfNotFound)
        elem.AddAppData(GetKey(), cache = new BRepCache);

    return cache;
    }
};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/2015
+===============+===============+===============+===============+===============+======*/
struct DrawHelper
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2015
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
* @bsimethod                                                    Brien.Bastings  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsGeometryVisible(ViewContextR context, Render::GeometryParamsCR geomParams, DRange3dCP range)
    {
    ViewFlags   viewFlags = context.GetViewFlags();

    switch (geomParams.GetGeometryClass())
        {
        case DgnGeometryClass::Construction:
            if (!viewFlags.ShowConstructions())
                return false;
            break;

        case DgnGeometryClass::Dimension:
            if (!viewFlags.ShowDimensions())
                return false;
            break;

        case DgnGeometryClass::Pattern:
            if (!viewFlags.ShowPatterns())
                return false;
            break;
        }

    if (nullptr == context.GetViewport())
        return true;

    if (nullptr != range && !range->IsNull())
        {
        if (!context.IsRangeVisible(*range))
            return false; // Sub-graphic outside range...
        }

    DgnSubCategory::Appearance appearance = context.GetViewport()->GetViewController().GetSubCategoryAppearance(geomParams.GetSubCategoryId());

    if (appearance.IsInvisible())
        return false;

    switch (context.GetDrawPurpose())
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2015
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static IBRepEntityPtr GetCachedSolidKernelEntity(ViewContextR context, DgnElementCP element, GeometryStreamEntryIdCR entryId)
    {
    if (nullptr == element)
        return nullptr;

    BRepCache* cache = BRepCache::Get(*element, false);

    if (nullptr == cache)
        return nullptr;

    BRepCache::IndexedGeomMap::const_iterator found = cache->m_map.find(entryId.GetIndex());

    if (found == cache->m_map.end())
        return nullptr;

    return found->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void SaveSolidKernelEntity(ViewContextR context, DgnElementCP element, GeometryStreamEntryIdCR entryId, IBRepEntityR entity)
    {
    // Only save for auto-locate, display has Render::Graphic, and other callers of Stroke should be ok reading again...
    if (nullptr == context.GetIPickGeom())
        return;

    if (nullptr == element)
        return;

    BRepCache* cache = BRepCache::Get(*element, true);

    cache->m_map[entryId.GetIndex()] = &entity;
    }

}; // DrawHelper

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryStreamIO::Collection::Draw(Render::GraphicBuilderR mainGraphic, ViewContextR context, Render::GeometryParamsR geomParams, bool activateParams, DgnElementCP element) const
    {
    bool geomParamsChanged = true;
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
                    bool strokeLineStyle = (activateParams && context.WantLineStyles() && geomParams.HasStrokedLineStyle());

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
                    bool strokeLineStyle = (activateParams && context.WantLineStyles() && geomParams.HasStrokedLineStyle());

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

                bool strokeLineStyle = (activateParams && context.WantLineStyles() && geomParams.HasStrokedLineStyle());

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
                bool strokeLineStyle = (activateParams && context.WantLineStyles() && geomParams.HasStrokedLineStyle());

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

                bool strokeLineStyle = (activateParams && context.WantLineStyles() && geomParams.HasStrokedLineStyle());

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

#if defined (BENTLEYCONFIG_PARASOLID) 
            case GeometryStreamIO::OpCode::ParasolidBRep:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                IBRepEntityPtr entityPtr = DrawHelper::GetCachedSolidKernelEntity(context, element, entryId);

                if (!entityPtr.IsValid())
                    {
                    if (!reader.Get(egOp, entityPtr))
                        break;

                    // Resolve/Cook face attachments...need to do this even when output isn't QVis because it's going to be cached...
                    IFaceMaterialAttachmentsCP attachments = entityPtr->GetFaceMaterialAttachments();

                    if (nullptr != attachments)
                        {
                        T_FaceAttachmentsVec const& faceAttachmentsVec = attachments->_GetFaceAttachmentsVec();

                        for (FaceAttachment const& attachment : faceAttachmentsVec)
                            attachment.CookFaceAttachment(context, geomParams);
                        }

                    DrawHelper::SaveSolidKernelEntity(context, element, entryId, *entityPtr);
                    }

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);
                currGraphic->AddBodyR(*entityPtr);
                break;
                }
#else
            case GeometryStreamIO::OpCode::ParasolidBRep:
                {
                // NOTE: Only update GeometryStreamEntryId from ParasolidBRep...could have multiple polyface if BRep had face attachments...
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);
                continue; // Don't exit loop in case we are in a sub-graphic...must add BRepPolyface or BRepCurveVector...
                }

            case GeometryStreamIO::OpCode::BRepPolyface:
                {
                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                PolyfaceQueryCarrier meshData(0, false, 0, 0, nullptr, nullptr);

                if (!BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier(egOp.m_data, meshData))
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);
                currGraphic->AddPolyface(meshData, false);
                break;
                };

            case GeometryStreamIO::OpCode::BRepCurveVector:
                {
                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                CurveVectorPtr curvePtr = BentleyGeometryFlatBuffer::BytesToCurveVector(egOp.m_data);

                if (!curvePtr.IsValid())
                    break;

                DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);
                currGraphic->AddCurveVectorR(*curvePtr, false);
                break;
                };
#endif

            case GeometryStreamIO::OpCode::TextString:
                {
                entryId.Increment();
                currGraphic->SetGeometryStreamEntryId(&entryId);

                if (!DrawHelper::IsGeometryVisible(context, geomParams, &subGraphicRange))
                    break;

                TextString  text;

                if (SUCCESS != TextStringPersistence::DecodeFromFlatBuf(text, egOp.m_data, egOp.m_dataSize, context.GetDgnDb()))
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

                if (nullptr == context.GetIPickGeom())
                    imagePtr->CreateTexture(context); // Draw border for pick as well as to avoid zombie even if texture fails...

                if (!imagePtr->GetUseFillTint())
                    {
                    Render::GeometryParams tintParams = geomParams;

                    tintParams.SetFillDisplay(FillDisplay::Always);
                    tintParams.SetFillColor(ColorDef(254, 255, 255)); // Don't use ColorDef::White() to avoid being affeceted by white on white reversal...

                    geomParamsChanged = true;
                    DrawHelper::CookGeometryParams(context, tintParams, *currGraphic, geomParamsChanged);
                    geomParamsChanged = true;
                    }
                else
                    {
                    DrawHelper::CookGeometryParams(context, geomParams, *currGraphic, geomParamsChanged);
                    }

                if (context.Is3dView())
                    imagePtr->AddToGraphic(*currGraphic);
                else
                    imagePtr->AddToGraphic2d(*currGraphic, geomParams.GetNetDisplayPriority());
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
* @bsimethod                                    Keith.Bentley                   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr GeometrySource::_Stroke(ViewContextR context, double pixelSize) const
    {
    return Draw(context, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr GeometrySource::Draw(ViewContextR context, double pixelSize) const
    {
    auto graphic = context.CreateWorldGraphic(GetPlacementTransform());
    Render::GeometryParams params;

    params.SetCategoryId(GetCategoryId());
    GeometryStreamIO::Collection(GetGeometryStream().GetData(), GetGeometryStream().GetSize()).Draw(*graphic, context, params, true, ToElement());

    return graphic->Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr GeometrySource::_StrokeHit(ViewContextR context, HitDetailCR hit) const
    {
    if (0 == hit.GetGeomDetail().GetGeometryStreamEntryId().GetIndex())
        return nullptr;

    switch (hit.GetSubSelectionMode())
        {
        case SubSelectionMode::Part:
            {
            if (hit.GetGeomDetail().GetGeometryStreamEntryId().GetGeometryPartId().IsValid())
                break;

            return nullptr;
            }

        case SubSelectionMode::Primitive:
            break;

        case SubSelectionMode::Segment:
            {
            if (nullptr != hit.GetGeomDetail().GetCurvePrimitive())
                break;

            return nullptr;
            }

        default:
            return nullptr;
        }

    // Get the GeometryParams for this hit from the GeometryStream...
    GeometryCollection collection(*this);
    Render::GraphicBuilderPtr graphic;

    for (auto iter : collection)
        {
        // Quick exclude of geometry that didn't generate the hit...
        if (hit.GetGeomDetail().GetGeometryStreamEntryId() != iter.GetGeometryStreamEntryId())
            continue;

        switch (hit.GetSubSelectionMode())
            {
            case SubSelectionMode::Part:
                {
                GeometryParams geomParams(iter.GetGeometryParams());

                graphic = context.CreateWorldGraphic(iter.GetSourceToWorld());
                context.AddSubGraphic(*graphic, iter.GetGeometryPartId(), iter.GetGeometryToSource(), geomParams);

                return graphic->Finish();
                }

            case SubSelectionMode::Segment:
                {
                GeometryParams geomParams(iter.GetGeometryParams()); // NOTE: Used for weight. A part can store weights in it's GeometryStream too, but this is probably good enough...
                GraphicParams  graphicParams;

                context.CookGeometryParams(geomParams, graphicParams); // Don't activate yet...need to tweak...
                graphicParams.SetWidth(graphicParams.GetWidth()+2); // NOTE: Would be nice if flashing made element "glow" for now just bump up weight...

                graphic = context.CreateWorldGraphic();
                graphic->ActivateGraphicParams(graphicParams);

                bool doSegmentFlash = (hit.GetHitType() < HitDetailType::Snap);

                if (!doSegmentFlash)
                    {
                    switch (static_cast<SnapDetailCR>(hit).GetSnapMode())
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

                DSegment3d      segment;
                CurveVectorPtr  curve;

                // Flash only the selected segment of linestrings/shapes based on snap mode...
                if (doSegmentFlash && hit.GetGeomDetail().GetSegment(segment))
                    curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLine(segment));
                else
                    curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, hit.GetGeomDetail().GetCurvePrimitive()->Clone());

                if (hit.GetViewport().Is3dView())
                    graphic->AddCurveVectorR(*curve, false);
                else
                    graphic->AddCurveVector2dR(*curve, false, geomParams.GetNetDisplayPriority());

                return graphic->Finish();
                }

            case SubSelectionMode::Primitive:
                {
                GeometricPrimitivePtr geom = iter.GetGeometryPtr();

                if (geom.IsValid())
                    {
                    if (!graphic.IsValid())
                        graphic = context.CreateWorldGraphic(iter.GetGeometryToWorld());

                    GeometryParams geomParams(iter.GetGeometryParams());

                    context.CookGeometryParams(geomParams, *graphic);
                    geom->AddToGraphic(*graphic);
                    break; // Keep going, want to draw all matching geometry (ex. multi-symb BRep is Polyface per-symbology)...
                    }

                DgnGeometryPartCPtr geomPart = iter.GetGeometryPartCPtr();

                if (!geomPart.IsValid())
                    return nullptr; // Shouldn't happen...

                GeometryCollection partCollection(geomPart->GetGeometryStream(), context.GetDgnDb());

                partCollection.SetNestedIteratorContext(iter); // Iterate part GeomStream in context of parent...

                for (auto partIter : partCollection)
                    {
                    // Quick exclude of part geometry that didn't generate the hit...pass true to compare part geometry index...
                    if (hit.GetGeomDetail().GetGeometryStreamEntryId(true) != partIter.GetGeometryStreamEntryId())
                        continue;

                    GeometricPrimitivePtr partGeom = partIter.GetGeometryPtr();

                    if (!partGeom.IsValid())
                        continue;

                    if (!graphic.IsValid())
                        graphic = context.CreateWorldGraphic(partIter.GetGeometryToWorld());

                    GeometryParams geomParams(partIter.GetGeometryParams());

                    context.CookGeometryParams(geomParams, *graphic);
                    partGeom->AddToGraphic(*graphic);
                    continue; // Keep going, want to draw all matching geometry (ex. multi-symb BRep is Polyface per-symbology)...
                    }

                return graphic->Finish(); // Done with part...
                }
            }
        }

    return graphic->Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus GeometrySource::_OnSnap(SnapContextR context) const
    {
    return context.DoDefaultDisplayableSnap();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2015
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

        case GeometryStreamIO::OpCode::BRepPolyface:
            return EntryType::Polyface;

        case GeometryStreamIO::OpCode::BRepCurveVector:
            return EntryType::CurveVector;

        case GeometryStreamIO::OpCode::TextString:
            return EntryType::TextString;

        case GeometryStreamIO::OpCode::Image:
            return EntryType::ImageGraphic;

        default:
            BeAssert(false); return EntryType::Unknown;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2015
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

            return (geom.IsValid() && !geom->GetAsCurveVector()->IsAnyRegionType()); // Accept "none" boundary type...
            }

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2015
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

            return (geom.IsValid() && geom->GetAsCurveVector()->IsAnyRegionType());
            }

        case GeometryStreamIO::OpCode::SolidPrimitive:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && !geom->GetAsISolidPrimitive()->GetCapped());
            }

        case GeometryStreamIO::OpCode::Polyface:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && !geom->GetAsPolyfaceHeader()->IsClosedByEdgePairing());
            }

        case GeometryStreamIO::OpCode::BsplineSurface:
            {
            return true;
            }

#if defined (BENTLEYCONFIG_PARASOLID)  
        case GeometryStreamIO::OpCode::ParasolidBRep:
            {
            auto ppfb = flatbuffers::GetRoot<FB::BRepData>(m_egOp.m_data);

            return (IBRepEntity::EntityType::Sheet == ((IBRepEntity::EntityType) ppfb->brepType()));
            }
#else
        case GeometryStreamIO::OpCode::BRepPolyface:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && !geom->GetAsPolyfaceHeader()->IsClosedByEdgePairing());
            }

        case GeometryStreamIO::OpCode::BRepCurveVector:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && geom->GetAsCurveVector()->IsAnyRegionType());
            }
#endif

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryCollection::Iterator::IsSolid() const
    {
    switch (m_egOp.m_opCode)
        {
        case GeometryStreamIO::OpCode::SolidPrimitive:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && geom->GetAsISolidPrimitive()->GetCapped());
            }

        case GeometryStreamIO::OpCode::Polyface:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && geom->GetAsPolyfaceHeader()->IsClosedByEdgePairing());
            }

#if defined (BENTLEYCONFIG_PARASOLID)  
        case GeometryStreamIO::OpCode::ParasolidBRep:
            {
            auto ppfb = flatbuffers::GetRoot<FB::BRepData>(m_egOp.m_data);

            return (IBRepEntity::EntityType::Solid == ((IBRepEntity::EntityType) ppfb->brepType()));
            }
#else
        case GeometryStreamIO::OpCode::BRepPolyface:
            {
            GeometricPrimitivePtr geom = GetGeometryPtr();

            return (geom.IsValid() && geom->GetAsPolyfaceHeader()->IsClosedByEdgePairing());
            }
#endif

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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

        uint32_t        opCode = *((uint32_t *) (m_data));
        uint32_t        dataSize = *((uint32_t *) (m_data + sizeof (opCode)));
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
#if defined (BENTLEYCONFIG_PARASOLID)
                if (!m_egOp.IsGeometryOp())
                    break;

                m_state->m_geomStreamEntryId.Increment();
#else
                if (!m_egOp.IsGeometryOp())
                    {
                    if (GeometryStreamIO::OpCode::ParasolidBRep == m_egOp.m_opCode)
                        m_state->m_geomStreamEntryId.Increment(); // NOTE: Only update GeometryStreamEntryId from ParasolidBRep...could have multiple polyface if BRep had face attachments...
                    break;
                    }

                if (!(GeometryStreamIO::OpCode::BRepPolyface == m_egOp.m_opCode || GeometryStreamIO::OpCode::BRepCurveVector == m_egOp.m_opCode))
                    m_state->m_geomStreamEntryId.Increment();
#endif

                m_state->m_geometry = nullptr; // Defer extract until asked...

                if (m_state->m_geomParams.GetCategoryId().IsValid())
                    m_state->m_geomParams.Resolve(m_state->m_dgnDb); // Resolve sub-category appearance...
                return;
                }
            }

        } while (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryCollection::SetNestedIteratorContext(Iterator const& iter)
    {
    m_state.m_geomParams = iter.m_state->m_geomParams;
    m_state.m_geomStreamEntryId = iter.m_state->m_geomStreamEntryId;
    m_state.m_sourceToWorld = iter.m_state->m_sourceToWorld;
    m_state.m_geomToSource = iter.m_state->m_geomToSource;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryCollection::GeometryCollection(GeometrySourceCR source) : m_state(source.GetSourceDgnDb())
    {
    m_data = source.GetGeometryStream().GetData();
    m_dataSize = source.GetGeometryStream().GetSize();
    m_state.m_geomParams.SetCategoryId(source.GetCategoryId());
    m_state.m_sourceToWorld = source.GetPlacementTransform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
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
* @bsimethod                                                    Brien.Bastings  11/2015
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
                entryId.SetIndex(entryId.GetIndex()+1);
                break;
                }

            default:
                {
                if (!egOp.IsGeometryOp())
                    break;

                entryId.SetGeometryPartId(DgnGeometryPartId());
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
BentleyStatus GeometryBuilder::GetGeometryStream(GeometryStreamR geom)
    {
    if (0 == m_writer.m_buffer.size())
        return ERROR;

    geom.SaveData(&m_writer.m_buffer.front(), (uint32_t) m_writer.m_buffer.size());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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

    DgnElementCP el = source.ToElement();

    if (nullptr != el && el->GetElementHandler()._IsRestrictedAction(DgnElement::RestrictedAction::SetGeometry))
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
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(DgnSubCategoryId subCategoryId)
    {
    GeometryParams elParams;

    elParams.SetCategoryId(m_elParams.GetCategoryId()); // Preserve current category...
    elParams.SetSubCategoryId(subCategoryId);

    return Append(elParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
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

    m_elParams = elParams;
    m_appearanceChanged = true; // Defer append until we actually have some geometry...

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(DgnGeometryPartId geomPartId, TransformCR geomToElement)
    {
    DRange3d    localRange;

    if (SUCCESS != DgnGeometryPart::QueryGeometryPartRange(localRange, m_dgnDb, geomPartId))
        return false; // part probably doesn't exist...

    return Append(geomPartId, geomToElement, localRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(DgnGeometryPartId geomPartId, DPoint3dCR origin, YawPitchRollAngles const& angles)
    {
    Transform   geomToElement = angles.ToTransform(origin);

    return Append(geomPartId, geomToElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(DgnGeometryPartId geomPartId, DPoint2dCR origin, AngleInDegrees const& angle)
    {
    Transform   geomToElement;

    geomToElement.InitFromOriginAngleAndLengths(origin, angle.Radians(), 1.0, 1.0);

    return Append(geomPartId, geomToElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryBuilder::OnNewGeom(DRange3dCR localRangeIn, bool isSubGraphic, GeometryStreamIO::OpCode opCode)
    {
    DRange3d localRange = localRangeIn;

    // NOTE: Expand range to include line style width. Maybe this should be removed when/if we start doing locate from mesh tiles...
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
            if (nullptr != m_elParams.GetGradient())
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
            m_writer.Append(m_elParamsModified, m_isPartCreate, m_is3d);
            m_appearanceChanged = m_appearanceModified = true;
            }
        }
    else if (m_appearanceChanged)
        {
        m_writer.Append(m_elParams, m_isPartCreate, m_is3d);
        m_appearanceChanged = m_appearanceModified = false;
        }

    if (isSubGraphic && !m_isPartCreate)
        m_writer.Append(localRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::AppendWorld(GeometricPrimitiveR geom)
    {
    if (!ConvertToLocal(geom))
        return false;

    return AppendLocal(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
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

#if defined (BENTLEYCONFIG_PARASOLID)
    GeometricPrimitivePtr clone;

    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    if (GeometricPrimitive::GeometryType::BRepEntity == geom.GetGeometryType())
        clone = GeometricPrimitive::Create(PSolidUtil::InstanceEntity(*geom.GetAsIBRepEntity()));
    else
        clone = geom.Clone();
#else
    GeometricPrimitivePtr clone = geom.Clone();
#endif

    return AppendWorld(*clone);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                   Earlin.Lutz 03/2016
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
* @bsimethod                                                    Brien.Bastings  04/2015
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

#if defined (BENTLEYCONFIG_PARASOLID)
    // NOTE: Avoid un-necessary copy of BRep. We just need to change entity transform...
    IBRepEntityPtr clone = PSolidUtil::InstanceEntity(geom);
#else
    IBRepEntityPtr clone = geom.Clone();
#endif

    GeometricPrimitivePtr geomPtr = GeometricPrimitive::Create(clone);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
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

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
//! Allows for the ViewContext-based TextAnnotationDraw to be used with an GeometryBuilder.
// @bsiclass                                                    Jeff.Marker     09/2015
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
// @bsimethod                                                   Jeff.Marker     09/2015
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
// @bsimethod                                                   Jeff.Marker     09/2015
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
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
void TextAnnotationDrawToGeometricPrimitive::_OutputGraphics(ViewContextR context)
    {
    TextAnnotationDraw annotationDraw(m_text);
    Render::GeometryParams geomParams(m_builder.GetGeometryParams());
    auto graphic = context.CreateWorldGraphic();

    annotationDraw.Draw(*graphic, context, geomParams);
    graphic->Finish();
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(TextAnnotationCR text, CoordSystem coord)
    {
    TextAnnotationDrawToGeometricPrimitive annotationDraw(text, *this, coord);

    GeometryProcessor::Process(annotationDraw, m_dgnDb);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryBuilder::Append(ImageGraphicCR geom, CoordSystem coord)
    {
    if (CoordSystem::Local == coord)
        {
        DRange3d localRange;

        if (!getRange(geom, localRange, nullptr))
            return false;

        OnNewGeom(localRange, m_appendAsSubGraphics, GeometryStreamIO::OpCode::Image);
        m_writer.Append(geom);

        return true;
        }

    GeometricPrimitivePtr geomPtr = GeometricPrimitive::Create(geom);

    return AppendWorld(*geomPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilder::GeometryBuilder(DgnDbR dgnDb, DgnCategoryId categoryId, Placement3dCR placement)
  : GeometryBuilder(dgnDb, categoryId, true)
    {
    m_placement3d = placement;
    m_placement3d.GetElementBoxR().Init(); // throw away pre-existing bounding box...
    m_havePlacement = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilder::GeometryBuilder(DgnDbR dgnDb, DgnCategoryId categoryId, Placement2dCR placement)
  : GeometryBuilder(dgnDb, categoryId, false)
    {
    m_placement2d = placement;
    m_placement2d.GetElementBoxR().Init(); // throw away pre-existing bounding box...
    m_havePlacement = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilder::GeometryBuilder(DgnDbR dgnDb, DgnCategoryId categoryId, bool is3d)
  : m_isPartCreate(!categoryId.IsValid()), m_is3d(is3d), m_dgnDb(dgnDb), m_writer(dgnDb)
    {
    m_elParams.SetCategoryId(categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  10/2016
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
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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

    // NOTE: YawPitchRollAngles::TryFromRotMatrix compares against Angle::SmallAngle, which after
    //       consulting with Earlin is too strict for our purposes and shouldn't be considered a failure.
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
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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
* @bsimethod                                                    Brien.Bastings  04/2015
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
