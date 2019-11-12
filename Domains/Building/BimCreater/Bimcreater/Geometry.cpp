/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "dgnplatform\TextString.h"


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
BentleyStatus GeometricTools::CreateGeometry(Dgn::PhysicalElementPtr element, BuildingPhysical::BuildingPhysicalModelR model, Dgn::DgnCategoryId categoryId)
    {
    Dgn::DgnDbR db = model.GetDgnDb();
    Dgn::DgnModelId modelId = model.GetModelId();
  //  Dgn::DgnCategoryId    categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalCategoryId(db, element->GetElementClass()->GetName().c_str());


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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreatePidLineGeometry(Dgn::DrawingGraphicR element, Dgn::DrawingModelR drawingModel, DPoint2dCP points, int count, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId)
	{
	Dgn::DgnDbR db = drawingModel.GetDgnDb();
	Dgn::DgnModelId modelId = drawingModel.GetModelId();
//	Dgn::DgnCategoryId    categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingDrawingCategoryId(db, PID_CATEGORY);


	Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
	if (!builder.IsValid())
		return BentleyStatus::BSIERROR;

	// Append geometry/params for tile casing

	builder->Append(categoryId);

	Dgn::Render::GeometryParams params;
	params.SetCategoryId(categoryId);
	params.SetSubCategoryId(subCategoryId);
	// params.SetMaterialId(ToyTileMaterial::QueryColoredPlasticMaterialId(db));
	params.SetLineColor(Dgn::ColorDef::Yellow());
	params.SetWeight(2);
	builder->Append(params);

	CurveVectorPtr line = CurveVector::CreateLinear(points, count, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	//DMatrix3d matrix;

	//matrix.multiply()  .TransformPoints(points, count);

	//DPoint3d arcPoints[3];
	//arcPoints[0] = DPoint3d::From( 5.0, 10.0, 0.0);
	//arcPoints[1] = DPoint3d::From( 0.0, 10.0, 0.0);
	//arcPoints[2] = DPoint3d::From(10.0, 10.0, 0.0);

	//DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY(arcPoints[0], 5.0);// FromArcCenterStartEnd(arcPoints[0], arcPoints[1], arcPoints[2]);
	//ellipse.SetSweep(0, PI);
	//ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc(ellipse);

 //   builder->Append(*arc);

	//Dgn::TextStringStylePtr style = Dgn::TextStringStyle::Create();
	//style->SetFont(Dgn::DgnFontManager::GetLastResortTrueTypeFont());
	//style->SetSize(DPoint2d::From(0.5, 0.5));


	//Dgn::TextStringPtr text = Dgn::TextString::Create();

	//text->SetText("Test");
	//text->SetStyle(*style);

	//builder->Append(*text);

	if (BentleyStatus::SUCCESS != builder->Finish(element))
		return  BentleyStatus::BSIERROR;

	return BentleyStatus::SUCCESS;

	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreatePidValveGeometry(Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId)
	{
	
	Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
	if (!builder.IsValid())
		return BentleyStatus::BSIERROR;

	// Append geometry/params for tile casing

	builder->Append(categoryId);

	Dgn::Render::GeometryParams params;
	params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
    params.SetLineColor(Dgn::ColorDef::Yellow());
	params.SetWeight(2);
	builder->Append(params);

	DPoint2d points[5];
	points[0] = DPoint2d::From( 0.0, -0.75);
	points[1] = DPoint2d::From( 0.0,  0.75);
	points[2] = DPoint2d::From( 3.0, -0.75);
	points[3] = DPoint2d::From( 3.0,  0.75);
	points[4] = DPoint2d::From( 0.0, -0.75);

	CurveVectorPtr line = CurveVector::CreateLinear(points, 5, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	if (BentleyStatus::SUCCESS != builder->Finish(element))
		return  BentleyStatus::BSIERROR;

	return BentleyStatus::SUCCESS;

	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreatePidPumpGeometry(Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId )
	{

	Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
	if (!builder.IsValid())
		return BentleyStatus::BSIERROR;

	// Append geometry/params for tile casing

	builder->Append(categoryId);

	Dgn::Render::GeometryParams params;
	params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
    params.SetLineColor(Dgn::ColorDef::Red());
	params.SetWeight(2);
	builder->Append(params);

	DPoint3d points[5];
	points[0] = DPoint3d::From(0.0, 0.0);

	DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY(points[0], 3.125);// FromArcCenterStartEnd(arcPoints[0], arcPoints[1], arcPoints[2]);
	ellipse.SetSweep(36.87 * PI/ 180, -306.87 * PI/180);
	ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc(ellipse);

	builder->Append(*arc);

	ellipse = DEllipse3d::FromCenterRadiusXY(points[0], .773);// FromArcCenterStartEnd(arcPoints[0], arcPoints[1], arcPoints[2]);
	ellipse.SetSweep(0, 2.0*PI);
	arc = ICurvePrimitive::CreateArc(ellipse);

	builder->Append(*arc);

	points[0] = DPoint3d::From(  0.0,    3.125);
	points[1] = DPoint3d::From(  4.637,  3.125);
	points[2] = DPoint3d::From(  4.637,  1.875);
	points[3] = DPoint3d::From(  2.5,    1.875);

	CurveVectorPtr line = CurveVector::CreateLinear(points, 4, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	points[0] = DPoint3d::From( -3.090, -3.517);
	points[1] = DPoint3d::From(  3.090, -3.517);
	points[2] = DPoint3d::From(  3.090, -3.903);
	points[3] = DPoint3d::From( -3.090, -3.903);
	points[4] = DPoint3d::From( -3.090, -3.517);

	line = CurveVector::CreateLinear(points, 5, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	points[0] = DPoint3d::From( -2.317, -3.517);
	points[1] = DPoint3d::From( -1.681, -2.632);

	line = CurveVector::CreateLinear(points, 2, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	points[0] = DPoint3d::From( 2.317, -3.517);
	points[1] = DPoint3d::From( 1.681, -2.632);

	line = CurveVector::CreateLinear(points, 2, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	if (BentleyStatus::SUCCESS != builder->Finish(element))
		return  BentleyStatus::BSIERROR;

	return BentleyStatus::SUCCESS;

	}


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreatePidRoundTankGeometry(Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId)
	{

	Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
	if (!builder.IsValid())
		return BentleyStatus::BSIERROR;

	// Append geometry/params for tile casing

	builder->Append(categoryId);

	Dgn::Render::GeometryParams params;
	params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
    params.SetLineColor(Dgn::ColorDef::Red());
	params.SetWeight(2);
	builder->Append(params);

	DPoint3d points[2];
	points[0] = DPoint3d::From(0.0, 0.0);

	DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY(points[0], 10.0);// FromArcCenterStartEnd(arcPoints[0], arcPoints[1], arcPoints[2]);
	ellipse.SetSweep(0, 2.0*PI);
	ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc(ellipse);

	if (!arc.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*arc);

	points[0] = DPoint3d::From(-10.0, 0.0);
	points[1] = DPoint3d::From(-10.0, -12.5);

	CurveVectorPtr line = CurveVector::CreateLinear(points, 2, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	points[0] = DPoint3d::From(10.0, 0.0);
	points[1] = DPoint3d::From(10.0, -12.5);

	line = CurveVector::CreateLinear(points, 2, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	if (BentleyStatus::SUCCESS != builder->Finish(element))
		return  BentleyStatus::BSIERROR;

	return BentleyStatus::SUCCESS;

	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreatePidTankGeometry(Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId)
	{

	Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
	if (!builder.IsValid())
		return BentleyStatus::BSIERROR;

	// Append geometry/params for tile casing

	builder->Append(categoryId);

	Dgn::Render::GeometryParams params;
	params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
    params.SetLineColor(Dgn::ColorDef::Red());
	params.SetWeight(2);
	builder->Append(params);

	DPoint3d points[7];
	points[0] = DPoint3d::From(  0.0,   26.25);
	points[1] = DPoint3d::From(  0.0,    0.0);
	points[2] = DPoint3d::From( 31.25,   0.0);
	points[3] = DPoint3d::From( 31.25,  26.25);
	points[4] = DPoint3d::From(  0.0,   26.25);
	points[5] = DPoint3d::From( 15.625, 34.063);
	points[6] = DPoint3d::From( 31.25,  26.25);

	CurveVectorPtr line = CurveVector::CreateLinear(points, 7, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	if (BentleyStatus::SUCCESS != builder->Finish(element))
		return  BentleyStatus::BSIERROR;

	return BentleyStatus::SUCCESS;

	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreatePidNozzleGeometry(Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId)
	{

	Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
	if (!builder.IsValid())
		return BentleyStatus::BSIERROR;

	// Append geometry/params for tile casing

	builder->Append(categoryId);

	Dgn::Render::GeometryParams params;
	params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
	params.SetLineColor(Dgn::ColorDef::Red());
	params.SetWeight(2);
	builder->Append(params);

	DPoint3d points[6];
	points[0] = DPoint3d::From(0.0,    0.252);
	points[1] = DPoint3d::From(0.752,  0.252);
	points[2] = DPoint3d::From(0.752,  0.5);
	points[3] = DPoint3d::From(0.752, -0.5);
	points[4] = DPoint3d::From(0.752, -0.252);
	points[5] = DPoint3d::From(0.0, -0.252);

	CurveVectorPtr line = CurveVector::CreateLinear(points, 6, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	if (BentleyStatus::SUCCESS != builder->Finish(element))
		return  BentleyStatus::BSIERROR;

	return BentleyStatus::SUCCESS;

	}


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreatePidVirtualNozzleGeometry(Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId)
    {

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
    if (!builder.IsValid())
        return BentleyStatus::BSIERROR;

    // Append geometry/params for tile casing

    builder->Append(categoryId);

    Dgn::Render::GeometryParams params;
    params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
    params.SetLineColor(Dgn::ColorDef::Red());
    params.SetFillColor(Dgn::ColorDef::Red());
    params.SetFillDisplay(BENTLEY_NAMESPACE_NAME::Dgn::Render::FillDisplay::Always);
    params.SetWeight(2);
    builder->Append(params);

    DPoint3d point;
    point = DPoint3d::From(0.0, 0.0);

    DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY(point, 0.2);
    ellipse.SetSweep(0, 2.0*PI);
    //ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc(ellipse);
    CurveVectorPtr arc = CurveVector::CreateDisk(ellipse, CurveVector::BOUNDARY_TYPE_Outer);

    if (!arc.IsValid())
        return  BentleyStatus::BSIERROR;

    builder->Append(*arc);

    if (BentleyStatus::SUCCESS != builder->Finish(element))
        return  BentleyStatus::BSIERROR;

    return BentleyStatus::SUCCESS;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreatePidReducerGeometry(Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId)
	{

	Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
	if (!builder.IsValid())
		return BentleyStatus::BSIERROR;

	// Append geometry/params for tile casing

	builder->Append(categoryId);

	Dgn::Render::GeometryParams params;
	params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
	params.SetLineColor(Dgn::ColorDef::Yellow());
	params.SetWeight(2);
	builder->Append(params);

	DPoint3d points[6];
	points[0] = DPoint3d::From(0.0,  0.750);
	points[1] = DPoint3d::From(1.5,  0.375);
	points[2] = DPoint3d::From(1.5, -0.375);
	points[3] = DPoint3d::From(0.0, -0.750);
	points[4] = DPoint3d::From(0.0,  0.750);
	points[5] = DPoint3d::From(0.0, -0.252);

	CurveVectorPtr line = CurveVector::CreateLinear(points, 6, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	if (BentleyStatus::SUCCESS != builder->Finish(element))
		return  BentleyStatus::BSIERROR;

	return BentleyStatus::SUCCESS;

	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreatePid3WayValveGeometry(Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId)
	{

	Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
	if (!builder.IsValid())
		return BentleyStatus::BSIERROR;

	// Append geometry/params for tile casing

	builder->Append(categoryId);

	Dgn::Render::GeometryParams params;
	params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
	params.SetLineColor(Dgn::ColorDef::Yellow());
	params.SetWeight(2);
	builder->Append(params);

	DPoint2d points[9];
	points[0] = DPoint2d::From(0.0, -0.75);
	points[1] = DPoint2d::From(0.0,  0.75);
	points[2] = DPoint2d::From(1.5,  0.0);
	points[3] = DPoint2d::From(0.75, 1.5);
	points[4] = DPoint2d::From(2.25, 1.5);
	points[5] = DPoint2d::From(0.75, -1.5);
	points[6] = DPoint2d::From(2.25, -1.5);
	points[7] = DPoint2d::From(1.5, 0.0);
	points[8] = DPoint2d::From(0.0, -0.75);

	CurveVectorPtr line = CurveVector::CreateLinear(points, 9, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);

	if (BentleyStatus::SUCCESS != builder->Finish(element))
		return  BentleyStatus::BSIERROR;

	return BentleyStatus::SUCCESS;

	}


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreatePidVesselGeometry(Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Dgn::DgnSubCategoryId subCategoryId)
	{

	Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
	if (!builder.IsValid())
		return BentleyStatus::BSIERROR;

	// Append geometry/params for tile casing

	builder->Append(categoryId);

	Dgn::Render::GeometryParams params;
	params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
    params.SetLineColor(Dgn::ColorDef::Red());
	params.SetWeight(2);
	builder->Append(params);

	DPoint3d points[5];
	points[0] = DPoint3d::From(0.0, 0.0);
	points[1] = DPoint3d::From(0.0, 5.0);
	points[2] = DPoint3d::From(2.5, 0.0);

	DEllipse3d ellipse = DEllipse3d::FromPoints(points[0], points[1], points[2], 0, -PI);
	ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc(ellipse);

	if (!arc.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*arc);

	points[0] = DPoint3d::From(15.0, 0.0);
	points[1] = DPoint3d::From(15.0, 5.0);
	points[2] = DPoint3d::From(17.5, 0.0);

	ellipse = DEllipse3d::FromPoints(points[0], points[1], points[2], 0, PI);
	arc = ICurvePrimitive::CreateArc(ellipse);

	if (!arc.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*arc);


	points[0] = DPoint3d::From(0.0, -5.0);
	points[1] = DPoint3d::From(0.0,  5.0);
	points[2] = DPoint3d::From(15.0, 5.0);
	points[3] = DPoint3d::From(15.0,-5.0);
	points[4] = DPoint3d::From(0.0, -5.0);

	CurveVectorPtr line = CurveVector::CreateLinear(points, 5, CurveVector::BOUNDARY_TYPE_None, true);
	if (!line.IsValid())
		return  BentleyStatus::BSIERROR;

	builder->Append(*line);



	if (BentleyStatus::SUCCESS != builder->Finish(element))
		return  BentleyStatus::BSIERROR;

	return BentleyStatus::SUCCESS;

	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::CreateAnnotationTextGeometry(Dgn::DrawingGraphicR element, Dgn::DgnCategoryId categoryId, Utf8StringCR textValue, Dgn::DgnSubCategoryId subCategoryId)
    {


    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(element);
    if (!builder.IsValid())
        return BentleyStatus::BSIERROR;

    // Append geometry/params for tile casing

    builder->Append(categoryId);

    Dgn::Render::GeometryParams params;
    params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
    params.SetLineColor(Dgn::ColorDef::White());
    params.SetWeight(2);
    builder->Append(params);


    Dgn::TextStringStylePtr style = Dgn::TextStringStyle::Create();
    style->SetFont(Dgn::DgnFontManager::GetLastResortTrueTypeFont());
    style->SetSize(DPoint2d::From(0.7, 0.7));
    style->SetIsBold(true);
    style->SetIsUnderlined(true);

    Dgn::TextStringPtr text = Dgn::TextString::Create();

    text->SetText(textValue.c_str());
    text->SetStyle(*style);

    DPoint3d pt = DPoint3d::From(0, 0, 0);
    text->SetOriginFromJustificationOrigin(pt, Dgn::TextString::HorizontalJustification::Center, Dgn::TextString::VerticalJustification::Middle);
    

    builder->Append(*text);

    if (BentleyStatus::SUCCESS != builder->Finish(element))
        return  BentleyStatus::BSIERROR;

    return BentleyStatus::SUCCESS;

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus GeometricTools::Create3dPipeGeometry(Dgn::PhysicalElementR pipe, Dgn::DgnCategoryId categoryId, double length, double diameter)
    {

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(pipe);
    if (!builder.IsValid())
        return BentleyStatus::BSIERROR;


    Dgn::Render::GeometryParams params;
    params.SetCategoryId(categoryId);
    params.SetLineColor(Dgn::ColorDef::Green());
    params.SetFillColor(Dgn::ColorDef::Green());
    builder->Append(params);

    // The pipe will be just a Cylinder along the x axis. The calling app will transform into world location
    DPoint3d points[2];

    points[0] = DPoint3d::From(0, 0, 0);
    points[1] = DPoint3d::From(length, 0, 0);

    DgnConeDetail coneDetail(points[0], points[1], diameter / 2.0, diameter / 2.0, false);

    ISolidPrimitivePtr pipeCylinder = ISolidPrimitive::CreateDgnCone(coneDetail);

    if (!pipeCylinder.IsValid())
        return  BentleyStatus::BSIERROR;

    builder->Append(*pipeCylinder);

    // Create the centerline

    Dgn::DgnSubCategoryId subCategoryId;

    BuildingDomain::BuildingDomainUtilities::FindOrCreateSubCategory(&pipe.GetDgnDb(), categoryId, subCategoryId, "Center Line");

    params.SetSubCategoryId(subCategoryId);
    params.SetWeight(2);
    builder->Append(params);

    CurveVectorPtr line = CurveVector::CreateLinear(points, 2, CurveVector::BOUNDARY_TYPE_None, true);
    if (!line.IsValid())
        return  BentleyStatus::BSIERROR;

    builder->Append(*line);

    if (BentleyStatus::SUCCESS != builder->Finish(pipe))
        return  BentleyStatus::BSIERROR;

    return BentleyStatus::SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr GeometricTools::CreateContainmentBuildingGeometry(/*Dgn::DgnCategoryId categoryId,*/ double radius, double height)
    {

//    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(pipe);
//    if (!builder.IsValid())
//        return BentleyStatus::BSIERROR;


   // Dgn::Render::GeometryParams params;
  //  params.SetCategoryId(categoryId);
 //   params.SetLineColor(Dgn::ColorDef::Green());
  //  params.SetFillColor(Dgn::ColorDef::Green());
  //  builder->Append(params);

    DPoint3d points[5];
    points[0] = DPoint3d::From(0.0,    0.0);
    points[1] = DPoint3d::From(0.0,    radius);
    points[2] = DPoint3d::From(radius, 0.0);

    DEllipse3d ellipse = DEllipse3d::FromPoints(points[0], points[1], points[2], 0, 2.0*PI);
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc(ellipse);

    if (!arc.IsValid())
        return  nullptr;

    return arc;


    // The pipe will be just a Cylinder along the x axis. The calling app will transform into world location
    //DPoint3d points[2];

    //points[0] = DPoint3d::From(0, 0, 0);
    //points[1] = DPoint3d::From(height, 0, 0);

    //DgnConeDetail coneDetail(points[0], points[1], radius, radius, false);

    //ISolidPrimitivePtr cylinder = ISolidPrimitive::CreateDgnCone(coneDetail);

    //if (!cylinder.IsValid())
    //    return  nullptr;

    //return cylinder;
    }


    
