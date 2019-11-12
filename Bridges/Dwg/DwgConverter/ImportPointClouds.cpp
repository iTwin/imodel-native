/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"


USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
class PointsCollector : public IPointsProcessor
{
private:
    ProtocolExtensionContext&   m_context;
    Render::GeometryParams&     m_geometryParams;
    GeometryBuilderR            m_geometryBuilder;
    Transform                   m_worldToElement;
    double                      m_pointDisplaySize;
    size_t                      m_totalCollected;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
explicit PointsCollector (ProtocolExtensionContext& context, GeometryBuilderR builder, Render::GeometryParams& params)
    : m_context(context), m_geometryBuilder(builder), m_geometryParams(params)
    {
    m_worldToElement.InitIdentity ();
    m_totalCollected = 0;
    m_pointDisplaySize = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetPointDisplaySize (double size)
    {
    m_pointDisplaySize = size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetElementTransformation (TransformCR trans)
    {
    m_worldToElement = trans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  GetNumCollected () const
    {
    return  m_totalCollected;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IPointsProcessor::Status    _Process (PointCloudDataQueryCP dataQuery) override
    {
    // query points
    size_t  numPoints = 0;
    if (nullptr == dataQuery || 0 == (numPoints = dataQuery->GetNumPoints()))
        return  IPointsProcessor::Status::Abort;

    double const*   points = dataQuery->GetPointsAsDoubles ();
    if (nullptr == points)
        return  IPointsProcessor::Status::Abort;

    // query colors in RGBA
    PointCloudDataQuery::Rgba const* colors = dataQuery->GetColors ();

    Render::GeometryParams  display(m_geometryParams);
    ColorDef                dgnColor = display.GetLineColor ();

    for (size_t i = 0; i < numPoints; i++)
        {
        size_t      pointIndex = 3 * i;
        DPoint3d    point1 = DPoint3d::From (points[pointIndex], points[pointIndex + 1], points[pointIndex+2]);
        DPoint3d    point2 = DPoint3d::From (point1.x + m_pointDisplaySize, point1.y, point1.z);

        ICurvePrimitivePtr  line = ICurvePrimitive::CreateLine (DSegment3d::From(point1, point2));
        
        line->TransformInPlace (m_worldToElement);

        m_geometryBuilder.Append (*line.get());

        if (nullptr != colors)
            {
            dgnColor.SetColors (colors[i][0], colors[i][1], colors[i][2], colors[i][3]);
            display.SetLineColor (dgnColor);
            }
        m_geometryBuilder.Append (display);
        }

    m_totalCollected += numPoints;

    return  IPointsProcessor::Status::Continue;
    }
};  // PointsCollector

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgPointCloudExExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporter& importer)
    {
    DwgDbEntityPtr&     entity = context.GetEntityPtrR ();
    DwgDbPointCloudExCP pointcloud = DwgDbPointCloudEx::Cast(entity.get());
    if (nullptr == pointcloud)
        return  BSIERROR;

    // if user does not want to import point clouds, stop here:
    if (!importer.GetOptions().GetImportPointClouds())
        return  BSISUCCESS;

    // WIP - point cloud is not currently supported by OpenDWG - worldDraw it.
    DgnModelR   targetModel = context.GetModel ();
    if (!pointcloud->IsSupported())
        return importer._ImportEntity (context.GetElementResultsR(), context.GetElementInputsR());

    DwgImporter::ElementCreateParams    createParams(targetModel);
    if (BSISUCCESS != importer._GetElementCreateParams(createParams, context.GetTransform(), context.GetEntity()))
        return  BSIERROR;

    // NEEDSWORK - points are not displayed well! Not even with small segments at this time!
    double      pointSize = 0.0;
    if (DwgDbStatus::Success != pointcloud->GetMinDistPrecision(pointSize))
        {
        DRange3d    range;
        pointcloud->GetNativeCloudExtent (range);
        pointSize = range.DiagonalDistance ();
        }

    // get the ECS
    Transform   ecs;
    pointcloud->GetEcs (ecs);

    RotMatrix   rotation = ecs.Matrix ();

    DPoint3d    placementPoint;
    ecs.GetTranslation (placementPoint);

    // remove the unit scale from the ECS
    double      unitScale = 1.0;
    if (context.GetTransform().IsRigidScale(unitScale))
        {
        rotation.ScaleColumns (unitScale, unitScale, unitScale);
        placementPoint.Scale (unitScale);
        rotation.Multiply (placementPoint);
        }

    YawPitchRollAngles  angles;
    YawPitchRollAngles::TryFromRotMatrix (angles, rotation);

    // create a geometry builder with the translation and rotation:
    GeometryBuilderPtr  builder;
    if (targetModel.Is3d())
        builder = GeometryBuilder::Create (targetModel, createParams.GetCategoryId(), placementPoint, angles);
    else
        builder = GeometryBuilder::Create (targetModel, createParams.GetCategoryId(), DPoint2d::From(placementPoint), angles.GetYaw());

    if (!builder.IsValid())
        return  BSIERROR;

    // get default color from the entity
    ColorDef    dgnColor = this->GetDgnColor (*entity.get());

    Render::GeometryParams  display;
    display.SetCategoryId (createParams.GetCategoryId());
    display.SetSubCategoryId (createParams.GetSubCategoryId());
    display.SetGeometryClass (Render::DgnGeometryClass::Primary);
    display.SetLineColor (dgnColor);
    display.SetWeight (2);

    PointsCollector*  collector = new PointsCollector (context, *builder.get(), display);
    if (nullptr == collector)
        return  BSIERROR;

    int     lod = importer.GetOptions().GetPointCloudLevelOfDetails ();
    if (lod < 0)
        lod = 1;
    else if (lod > 100)
        lod = 100;

    collector->SetPointDisplaySize (pointSize);

    // scale the geometry by user scales:
    DVec3d  userScale;
    rotation.NormalizeColumnsOf (rotation, userScale);

    // translate the geometry from the world to the element:
    rotation.MultiplyTranspose (placementPoint);
    placementPoint.Negate ();

    ecs.InitFrom (RotMatrix::FromScaleFactors(userScale.x, userScale.y, userScale.z), placementPoint);

    collector->SetElementTransformation (ecs);

    DwgDbStatus status = pointcloud->TraversePointData (collector, nullptr, PointCloudDataQuery::Type::Color, lod);

    // create an element from the geometry builder w/points collected
    if (DwgDbStatus::Success == status && collector->GetNumCollected() > 0)
        {
        ElementFactory  factory(context.GetElementResultsR(), context.GetElementInputsR(), createParams, importer);
        factory.SetGeometryBuilder (builder.get());

        status = ToDwgDbStatus (factory.CreateElement());
        }

    delete collector;

    return  DwgDbStatus::Success == status ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef        DwgPointCloudExExt::GetDgnColor (DwgDbEntityCR entity) const
    {
    DwgCmEntityColor    color = entity.GetEntityColor ();
    if (color.IsByLayer())
        {
        DwgDbLayerTableRecordPtr    layer(entity.GetLayerId(), DwgDbOpenMode::ForRead);
        if (!layer.IsNull())
            color = layer->GetColor ();
        }
    else if (color.IsByBlock())
        {
        return  ColorDef::White();
        }

    if (color.IsByACI())
        return  DwgHelper::GetColorDefFromACI (color.GetIndex());
    else
        return ColorDef (color.GetRed(), color.GetGreen(), color.GetBlue());
    }

