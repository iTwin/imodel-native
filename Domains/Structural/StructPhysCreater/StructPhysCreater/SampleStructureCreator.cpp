/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/SampleStructureCreator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "GeometricTools.h"
#include "SampleStructureCreator.h"


#pragma region PRIVATE_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SampleStructureCreator::CreateBeam12Geometry(ArchitecturalPhysical::ArchitecturalBaseElementPtr element, BuildingPhysical::BuildingPhysicalModelR model)
    {
    Dgn::ColorDef transBlue(0, 0, 0xff, 0x19);
    StructuralMemberGeometricProperties* properties = new StructuralMemberGeometricProperties(BEAM_WIDTH, BEAM_12_LENGTH, BEAM_DEPTH, transBlue, transBlue);
    return GeometricTools::CreateStructuralMemberGeometry(element, model, properties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SampleStructureCreator::CreateBeam15Geometry(ArchitecturalPhysical::ArchitecturalBaseElementPtr element, BuildingPhysical::BuildingPhysicalModelR model)
    {
    Dgn::ColorDef transBlue(0, 0, 0xff, 0x19);
    StructuralMemberGeometricProperties* properties = new StructuralMemberGeometricProperties(BEAM_15_LENGTH, BEAM_WIDTH, BEAM_DEPTH, transBlue, transBlue);
    return GeometricTools::CreateStructuralMemberGeometry(element, model, properties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SampleStructureCreator::CreateColumnGeometry(ArchitecturalPhysical::ArchitecturalBaseElementPtr element, BuildingPhysical::BuildingPhysicalModelR model)
    {
    Dgn::ColorDef transBlue(0, 0, 0xff, 0x19);
    StructuralMemberGeometricProperties* properties = new StructuralMemberGeometricProperties(COLUMN_WIDTH, COLUMN_DEPTH, COLUMN_HEIGHT, transBlue, transBlue);
    return GeometricTools::CreateStructuralMemberGeometry(element, model, properties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SampleStructureCreator::CreateSlabGeometry(ArchitecturalPhysical::ArchitecturalBaseElementPtr element, BuildingPhysical::BuildingPhysicalModelR model)
    {
    Dgn::ColorDef transRed(0xff, 0, 0, 0x19);
    StructuralMemberGeometricProperties* properties = new StructuralMemberGeometricProperties(SLAB_DEPTH, SLAB_WIDTH, SLAB_THICKNESS, transRed, transRed);
    return GeometricTools::CreateStructuralMemberGeometry(element, model, properties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SampleStructureCreator::CreateWallGeometry(ArchitecturalPhysical::ArchitecturalBaseElementPtr element, BuildingPhysical::BuildingPhysicalModelR model)
    {
    Dgn::ColorDef transRed(0xff, 0, 0, 0x19);
    StructuralMemberGeometricProperties* properties = new StructuralMemberGeometricProperties(WALL_WIDTH, WALL_THICKNESS, WALL_DEPTH, transRed, transRed);
    return GeometricTools::CreateStructuralMemberGeometry(element, model, properties);
    }

#pragma endregion


#pragma region PRIVATE_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SampleStructureCreator::CreateBeams(BuildingPhysical::BuildingPhysicalModelR physicalModel, ECN::ECClassP elementClass)
    {
    ArchitecturalPhysical::ArchitecturalBaseElementPtr buildingElement;
    Dgn::Placement3d placement;
    Dgn::DgnDbStatus status;

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(COLUMN_OFFSET, COLUMN_OFFSET, COLUMN_HEIGHT - BEAM_DEPTH);
    status = buildingElement->SetPlacement(placement);
    CreateBeam12Geometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(SLAB_DEPTH - BEAM_WIDTH + COLUMN_OFFSET, COLUMN_OFFSET, COLUMN_HEIGHT - BEAM_DEPTH);
    status = buildingElement->SetPlacement(placement);
    CreateBeam12Geometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(COLUMN_OFFSET, COLUMN_OFFSET, COLUMN_HEIGHT - BEAM_DEPTH);
    status = buildingElement->SetPlacement(placement);
    CreateBeam15Geometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(COLUMN_OFFSET, SLAB_WIDTH - BEAM_WIDTH + COLUMN_OFFSET, COLUMN_HEIGHT - BEAM_DEPTH);
    status = buildingElement->SetPlacement(placement);
    CreateBeam15Geometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SampleStructureCreator::CreateColumns(BuildingPhysical::BuildingPhysicalModelR physicalModel, ECN::ECClassP elementClass)
    {
    ArchitecturalPhysical::ArchitecturalBaseElementPtr buildingElement;
    Dgn::Placement3d placement;
    Dgn::DgnDbStatus status;

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(0, 0, 0);
    status = buildingElement->SetPlacement(placement);
    CreateColumnGeometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(SLAB_DEPTH, 0, 0);
    status = buildingElement->SetPlacement(placement);
    CreateColumnGeometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(SLAB_DEPTH, SLAB_WIDTH, 0);
    status = buildingElement->SetPlacement(placement);
    CreateColumnGeometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(0, SLAB_WIDTH, 0);
    status = buildingElement->SetPlacement(placement);
    CreateColumnGeometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SampleStructureCreator::CreateSlabs(BuildingPhysical::BuildingPhysicalModelR physicalModel, ECN::ECClassP elementClass)
    {
    ArchitecturalPhysical::ArchitecturalBaseElementPtr buildingElement;
    Dgn::Placement3d placement;
    Dgn::DgnDbStatus status;

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(COLUMN_OFFSET, COLUMN_OFFSET, COLUMN_HEIGHT);
    status = buildingElement->SetPlacement(placement);
    CreateSlabGeometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SampleStructureCreator::CreateWalls(BuildingPhysical::BuildingPhysicalModelR physicalModel, ECN::ECClassP elementClass)
    {
    ArchitecturalPhysical::ArchitecturalBaseElementPtr buildingElement;
    Dgn::Placement3d placement;
    Dgn::DgnDbStatus status;

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(COLUMN_OFFSET, WALL_OFFSET, 0);
    status = buildingElement->SetPlacement(placement);
    CreateWallGeometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    buildingElement = ArchitecturalPhysical::ArchitecturalBaseElement::Create(elementClass->GetName(), physicalModel);
    placement.GetOriginR() = DPoint3d::From(COLUMN_OFFSET, SLAB_WIDTH + WALL_OFFSET, 0);
    status = buildingElement->SetPlacement(placement);
    CreateWallGeometry(buildingElement, physicalModel);
    buildingElement->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

#pragma endregion
