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
BentleyStatus GeometricTools::CreateDoorGeometry(ArchitecturalPhysical::DoorPtr door, BuildingPhysical::BuildingPhysicalModelR model)
    {
    Dgn::DgnDbR db = model.GetDgnDb();
    Dgn::DgnModelId modelId = model.GetModelId();
    Dgn::DgnCategoryId categoryId = ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorCategoryId(db);

    YawPitchRollAngles angles = YawPitchRollAngles::FromDegrees(90, 0, 0);

    ECN::ECValue doubleValue;

    Dgn::DgnDbStatus status = door->SetPropertyValue("OverallWidth", 250.0);
    status = door->SetPropertyValue("OverallHeight", 250.0);

    Dgn::Placement3d placement;

    placement.GetAnglesR() = angles;
    placement.GetOriginR() = DPoint3d::From(100, 100, 100);

    status = door->SetPlacement(placement);

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*door);
    if (!builder.IsValid())
        return BentleyStatus::BSIERROR;

    // Append geometry/params for tile casing
    builder->Append(ArchitecturalPhysical::ArchitecturalPhysicalCategory::QueryBuildingPhysicalDoorFrameSubCategoryId(db));
 //   builder->Append(GetCasingMaterialParams(db, categoryId, casingMaterialType));

    // Units are in Meters. Door Frame would be about 75mm x 150mm

    double frameDepth = 0.075;
    double frameWidth = 0.150;
    double openingWidht = 0.9;
    double openingHeight = 2.150;

    DPoint3d points[4];
    points[0] = DPoint3d::From(-frameDepth, -frameWidth / 2.0, 0.0);
    points[1] = DPoint3d::From(0.0, -frameWidth / 2.0, 0.0);
    points[2] = DPoint3d::From(0.0, frameWidth / 2.0, 0.0);
    points[3] = DPoint3d::From(-frameDepth, frameWidth / 2.0, 0.0);

    DVec3d vec = DVec3d::From(0.0, 0.0, openingHeight);

    CurveVectorPtr shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    if (!shape.IsValid())
        return  BentleyStatus::BSIERROR;

    ISolidPrimitivePtr casing = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shape, vec, true));

    if (!casing.IsValid())
        return  BentleyStatus::BSIERROR;


    builder->Append(*casing);

    Transform matrix;
    matrix.InitFromOriginAndLengths(DPoint2d::From(openingWidht + frameDepth, 0.0), 1.0, 1.0);

    casing->TransformInPlace(matrix);

    builder->Append(*casing);


    vec = DVec3d::From(0.0, 0.0, openingWidht + (2.0*frameDepth));

    shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    if (!shape.IsValid())
        return  BentleyStatus::BSIERROR;

    casing = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shape, vec, true));

    if (!casing.IsValid())
        return  BentleyStatus::BSIERROR;

    matrix.InitFromOriginAndVectors(DPoint3d::From(-frameDepth, 0, openingHeight), DVec3d::From(0.0, 0.0, -1.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(1.0, 0.0, 0.0));

    casing->TransformInPlace(matrix);
    builder->Append(*casing);


    Dgn::DgnCode code = Dgn::CodeSpec::CreateCode(BENTLEY_ARCHITECTURAL_PHYSICAL_AUTHORITY, model, "D001");
    door->SetCode(code);

    angles = door->GetPropertyValueYpr("Yaw", "Pitch", "Roll");
    //points[0] = DPoint3d::From(-a, 0.0, 10.0);
    //points[1] = DPoint3d::From(a, 0.0, 10.0);
    //points[2] = DPoint3d::From(0.0, b, 10.0);

    ////vec = DVec3d::From(0.0, 0.0, 2);

    //shape = CurveVector::CreateLinear(points, _countof(points), CurveVector::BOUNDARY_TYPE_Outer, true);
    //if (!shape.IsValid())
    //    return nullptr;

    //casing = ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(shape, vec, true));

    //if (!casing.IsValid())
    //    return nullptr;


    //builder->Append(*casing);


    //// Append geometry/params for circular magnet
    //builder->Append(ToyTileCategory::QueryMagnetSubCategoryId(db));
    //builder->Append(GetMagnetMaterialParams(db, categoryId));
    //builder->Append(ToyTileGeometry::QueryCircularMagnetPartId(db), casingCenter);

    //DgnGeometryPartId magnetPartId = ToyTileGeometry::QueryRectangularMagnetPartId(db);
    //DPoint3d magnetSize = ToyTileGeometry::GetRectangularMagnetSize();
    //double magnetOffset = ToyTileGeometry::GetMagnetInset() + magnetSize.y/2;

    //// Append geometry/params for rectangular magnets
    //builder->Append(magnetPartId, DPoint3d::From(casingCenter.x, magnetOffset, casingCenter.z));
    //builder->Append(magnetPartId, DPoint3d::From(casingCenter.x, casingSize.y - magnetOffset, casingCenter.z));
    //builder->Append(magnetPartId, DPoint3d::From(magnetOffset, casingCenter.y, casingCenter.z), YawPitchRollAngles::FromDegrees(90.0, 0.0, 0.0));
    //builder->Append(magnetPartId, DPoint3d::From(casingSize.x - magnetOffset, casingCenter.y, casingCenter.z), YawPitchRollAngles::FromDegrees(90.0, 0.0, 0.0));

    if (BentleyStatus::SUCCESS != builder->Finish(*door))
        return  BentleyStatus::BSIERROR;

    return BentleyStatus::SUCCESS;
    }
