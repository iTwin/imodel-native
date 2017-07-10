/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchPhysCreater/ArchPhysCreater/Geometry.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreateFrameGeometry(Dgn::GeometryBuilderPtr builder, BuildingPhysical::BuildingPhysicalModelR model, double frameDepth, double frameWidth, double height, double width, bool fullFrame)
    {

    // Frame Depth is the dimension of the frame with the thickness of the wall.
    // Frame Width  is the dimension of the frame parallel with the wall. 
    // Height is the interior height of the frame
    // Width is the interior width of the frame.
    // fullFrame indicates the frame is for an item like a window that needs a bottom.
    // It is assumed that the frame is vertical. 

    DPoint3d points[4];
    points[0] = DPoint3d::From(-frameWidth ,         0.0, 0.0);
    points[1] = DPoint3d::From(        0.0,          0.0, 0.0);
    points[2] = DPoint3d::From(        0.0,   frameDepth, 0.0);
    points[3] = DPoint3d::From(-frameWidth,   frameDepth, 0.0);

    DVec3d vec = DVec3d::From(0.0, 0.0, height);

    CurveVectorPtr shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    if (!shape.IsValid())
        return  BentleyStatus::BSIERROR;

    ISolidPrimitivePtr frame = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shape, vec, true));

    if (!frame.IsValid())
        return  BentleyStatus::BSIERROR;

    if ( !builder->Append(*frame) )
        return  BentleyStatus::BSIERROR;

    Transform matrix;
    matrix.InitFromOriginAndLengths(DPoint2d::From(width + frameWidth, 0.0), 1.0, 1.0);

    frame->TransformInPlace(matrix);

    if (!builder->Append(*frame))
        return  BentleyStatus::BSIERROR;

    vec = DVec3d::From(0.0, 0.0, width + (2.0*frameWidth));

    shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    if (!shape.IsValid())
        return  BentleyStatus::BSIERROR;

    frame = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shape, vec, true));

    if (!frame.IsValid())
        return  BentleyStatus::BSIERROR;

    matrix.InitFromOriginAndVectors(DPoint3d::From(-frameWidth, 0, height), DVec3d::From(0.0, 0.0, -1.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(1.0, 0.0, 0.0));

    frame->TransformInPlace(matrix);

    if (!builder->Append(*frame))
        return  BentleyStatus::BSIERROR;


    if (fullFrame)
        {
        matrix.InitFromOriginAndVectors(DPoint3d::From(0, 0, -height - frameWidth), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));

        frame->TransformInPlace(matrix);

        if (!builder->Append(*frame))
            return  BentleyStatus::BSIERROR;
        }


    return BentleyStatus::SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreateDoorGeometry(Dgn::PhysicalElementPtr door, BuildingPhysical::BuildingPhysicalModelR model)
    {
    Dgn::DgnDbR db = model.GetDgnDb();
    Dgn::DgnModelId modelId = model.GetModelId();
    //Dgn::DgnCategoryId categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorCategoryId(db);


    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*door);
    if (!builder.IsValid())
        return BentleyStatus::BSIERROR;

    // Append geometry/params for tile casing

   // builder->Append(ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorFrameSubCategoryId(db));

    Dgn::Render::GeometryParams params;
//    params.SetCategoryId(categoryId);
 //   params.SetSubCategoryId(ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorFrameSubCategoryId(db));
    // params.SetMaterialId(ToyTileMaterial::QueryColoredPlasticMaterialId(db));
    params.SetLineColor(Dgn::ColorDef::Red());
    params.SetFillColor(Dgn::ColorDef::Red());
    builder->Append(params);

    // Units are in Meters. Door Frame would be about 75mm x 150mm

    double frameDepth     = 0.150;
    double frameWidth     = 0.075;
    double openingWidth   = 0.9;
    double openingHeight  = 2.150;

    CreateFrameGeometry( builder, model, frameDepth, frameWidth, openingHeight, openingWidth, false);

    double panelWidth      = openingWidth;
    double panelThickness  = 0.035;
    double panelBaseOffset = 0.035;
    double panelFaceOffset = frameDepth / 2.0;

    // Create the Panel

  //  builder->Append(ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorPanelSubCategoryId(db));

 //   params.SetCategoryId(categoryId);
  //  params.SetSubCategoryId(ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorPanelSubCategoryId(db));
    params.SetLineColor(Dgn::ColorDef::Brown());
    params.SetFillColor(Dgn::ColorDef::Brown());
    builder->Append(params);


    DPoint3d points[4];
    points[0] = DPoint3d::From( 0.0,        panelFaceOffset,                  panelBaseOffset);
    points[1] = DPoint3d::From( panelWidth, panelFaceOffset,                  panelBaseOffset);
    points[2] = DPoint3d::From( panelWidth, panelFaceOffset + panelThickness, panelBaseOffset);
    points[3] = DPoint3d::From( 0.0,        panelFaceOffset + panelThickness, panelBaseOffset);

    DVec3d vec = DVec3d::From(0.0, 0.0, openingHeight - panelBaseOffset);

    CurveVectorPtr shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    if (!shape.IsValid())
        return  BentleyStatus::BSIERROR;

    ISolidPrimitivePtr panel = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shape, vec, true));

    if (!panel.IsValid())
        return  BentleyStatus::BSIERROR;

    builder->Append(*panel);

    if (BentleyStatus::SUCCESS != builder->Finish(*door))
        return  BentleyStatus::BSIERROR;

    return BentleyStatus::SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreateWindowGeometry(Dgn::PhysicalElementPtr window, BuildingPhysical::BuildingPhysicalModelR model)
    {
    Dgn::DgnDbR db = model.GetDgnDb();
    Dgn::DgnModelId modelId = model.GetModelId();
    //Dgn::DgnCategoryId    windowCategoryId      = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalWindowCategoryId(db);
    //Dgn::DgnSubCategoryId windowFraneCategoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalWindowFrameSubCategoryId(db);
   // Dgn::DgnSubCategoryId windowPanelCategoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalWindowPanelSubCategoryId(db);


    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*window);
    if (!builder.IsValid())
        return BentleyStatus::BSIERROR;

    // Append geometry/params for tile casing

//    builder->Append(windowFraneCategoryId);

    Dgn::Render::GeometryParams params;
 //   params.SetCategoryId(windowCategoryId);
   // params.SetSubCategoryId(windowFraneCategoryId);
    // params.SetMaterialId(ToyTileMaterial::QueryColoredPlasticMaterialId(db));
    params.SetLineColor(Dgn::ColorDef::DarkGrey());
    params.SetFillColor(Dgn::ColorDef::DarkGrey());
    builder->Append(params);

    // Units are in Meters. Window Frame would be about 75mm x 150mm

    double frameDepth = 0.150;
    double frameWidth = 0.075;
    double openingWidth = 0.9;
    double openingHeight = 2.150;

    CreateFrameGeometry(builder, model, frameDepth, frameWidth, openingHeight, openingWidth, true);

    double panelWidth = openingWidth;
    double panelThickness = 0.035;
    double panelBaseOffset = 0.035;
    double panelFaceOffset = frameDepth / 2.0 - (panelThickness / 2.0);

    // Create the Panel

  //  builder->Append(windowPanelCategoryId);

 //   params.SetCategoryId(windowCategoryId);
  //  params.SetSubCategoryId(windowPanelCategoryId);
    Dgn::ColorDef color(255, 255, 255, 100);
    params.SetLineColor(color);
    params.SetFillColor(color);
    builder->Append(params);


    DPoint3d points[4];
    points[0] = DPoint3d::From(0.0,        panelFaceOffset, 0.0);
    points[1] = DPoint3d::From(panelWidth, panelFaceOffset, 0.0);
    points[2] = DPoint3d::From(panelWidth, panelFaceOffset + panelThickness, 0.0);
    points[3] = DPoint3d::From(0.0,        panelFaceOffset + panelThickness, 0.0);

    DVec3d vec = DVec3d::From(0.0, 0.0, openingHeight);

    CurveVectorPtr shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    if (!shape.IsValid())
        return  BentleyStatus::BSIERROR;

    ISolidPrimitivePtr panel = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shape, vec, true));

    if (!panel.IsValid())
        return  BentleyStatus::BSIERROR;

    builder->Append(*panel);

    if (BentleyStatus::SUCCESS != builder->Finish(*window))
        return  BentleyStatus::BSIERROR;

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreateGeometry(Dgn::PhysicalElementPtr element, BuildingPhysical::BuildingPhysicalModelR model)
    {
    Dgn::DgnDbR db = model.GetDgnDb();
    Dgn::DgnModelId modelId = model.GetModelId();
    Dgn::DgnCategoryId    categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalCategoryId(db, element->GetElementClass()->GetName().c_str());


    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*element);
    if (!builder.IsValid())
        return BentleyStatus::BSIERROR;

    // Append geometry/params for tile casing

    builder->Append(categoryId);

    Dgn::Render::GeometryParams params;
    params.SetCategoryId(categoryId);
    //params.SetSubCategoryId(windowFraneCategoryId);
    // params.SetMaterialId(ToyTileMaterial::QueryColoredPlasticMaterialId(db));
    params.SetLineColor(Dgn::ColorDef::DarkGrey());
    params.SetFillColor(Dgn::ColorDef::DarkGrey());
    builder->Append(params);

    // Units are in Meters. Window Frame would be about 75mm x 150mm

    double frameDepth = 0.150;
    double frameWidth = 0.075;
    double openingWidth = 0.9;
    double openingHeight = 2.150;

    CreateFrameGeometry(builder, model, frameDepth, frameWidth, openingHeight, openingWidth, true);

    double panelWidth = openingWidth;
    double panelThickness = 0.035;
    double panelBaseOffset = 0.035;
    double panelFaceOffset = frameDepth / 2.0 - (panelThickness / 2.0);

    // Create the Panel

//    builder->Append(windowPanelCategoryId);

//    params.SetCategoryId(windowCategoryId);
//    params.SetSubCategoryId(windowPanelCategoryId);
    Dgn::ColorDef color(255, 255, 255, 100);
    params.SetLineColor(color);
    params.SetFillColor(color);
    builder->Append(params);


    DPoint3d points[4];
    points[0] = DPoint3d::From(0.0, panelFaceOffset, 0.0);
    points[1] = DPoint3d::From(panelWidth, panelFaceOffset, 0.0);
    points[2] = DPoint3d::From(panelWidth, panelFaceOffset + panelThickness, 0.0);
    points[3] = DPoint3d::From(0.0, panelFaceOffset + panelThickness, 0.0);

    DVec3d vec = DVec3d::From(0.0, 0.0, openingHeight);

    CurveVectorPtr shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    if (!shape.IsValid())
        return  BentleyStatus::BSIERROR;

    ISolidPrimitivePtr panel = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shape, vec, true));

    if (!panel.IsValid())
        return  BentleyStatus::BSIERROR;

    builder->Append(*panel);

    if (BentleyStatus::SUCCESS != builder->Finish(*element))
        return  BentleyStatus::BSIERROR;

    return BentleyStatus::SUCCESS;
    }

