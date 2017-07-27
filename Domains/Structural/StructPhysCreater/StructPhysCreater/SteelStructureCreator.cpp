/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/SteelStructureCreator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"


#pragma region PROTECTED_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* SteelStructureCreator::GetBeam12Properties()
    {
    Dgn::ColorDef transCyan(0, 0xff, 0xff, 0x19);
    return new PhysicalProperties(BEAM_WIDTH, BEAM_DEPTH, BEAM_12_LENGTH, ShapeTools::Shape::I, transCyan, transCyan);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* SteelStructureCreator::GetBeam15Properties()
    {
    Dgn::ColorDef transCyan(0, 0xff, 0xff, 0x19);
    return new PhysicalProperties(BEAM_WIDTH, BEAM_DEPTH, BEAM_15_LENGTH, ShapeTools::Shape::I, transCyan, transCyan);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* SteelStructureCreator::GetColumnProperties()
    {
    Dgn::ColorDef transCyan(0, 0xff, 0xff, 0x19);
    return new PhysicalProperties(COLUMN_WIDTH, COLUMN_DEPTH, COLUMN_HEIGHT, ShapeTools::Shape::HSSRectangle, transCyan, transCyan);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* SteelStructureCreator::GetGussetPlateLargeProperties()
    {
    Dgn::ColorDef transMagenta(0xff, 0, 0xff, 0x19);
    return new PhysicalProperties(GUSSET_PLATE_LARGE_WIDTH, GUSSET_PLATE_LARGE_DEPTH, GUSSET_PLATE_LARGE_THICKNESS, ShapeTools::Shape::Rectangle, transMagenta, transMagenta);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* SteelStructureCreator::GetGussetPlateSmallProperties()
    {
    Dgn::ColorDef transMagenta(0xff, 0, 0xff, 0x19);
    return new PhysicalProperties(GUSSET_PLATE_SMALL_WIDTH, GUSSET_PLATE_SMALL_DEPTH, GUSSET_PLATE_SMALL_THICKNESS, ShapeTools::Shape::Rectangle, transMagenta, transMagenta);
    }

#pragma endregion


#pragma region PUBLIC_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SteelStructureCreator::CreateGussetPlates(StructuralPhysical::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass)
    {
    Dgn::PhysicalElementPtr element;
    PhysicalProperties* properties;
    Transform rotationMatrix;
    Transform linearMatrix;
    Dgn::DgnDbStatus status;

    // Gusset Plate Large
    element = StructuralDomain::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetGussetPlateLargeProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0), DVec3d::From(0.0, -1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(WALL_WIDTH / 2 - GUSSET_PLATE_LARGE_OFFSET,
                                                         COLUMN_OFFSET + BEAM_WIDTH / 2,
                                                         COLUMN_HEIGHT - BEAM_DEPTH - GUSSET_PLATE_LARGE_DEPTH),
                                          DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    // Gusset Plate Small
    element = StructuralDomain::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetGussetPlateSmallProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0), DVec3d::From(0.0, -1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_DEPTH,
                                                         COLUMN_OFFSET + BEAM_WIDTH / 2,
                                                         0.0),
                                          DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    // Gusset Plate Small
    element = StructuralDomain::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetGussetPlateSmallProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0), DVec3d::From(0.0, -1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(WALL_WIDTH - COLUMN_DEPTH - COLUMN_OFFSET,
                                                         COLUMN_OFFSET + BEAM_WIDTH / 2,
                                                         0.0),
                                          DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    // Gusset Plate Large
    element = StructuralDomain::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetGussetPlateLargeProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0), DVec3d::From(0.0, -1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(WALL_WIDTH / 2 - GUSSET_PLATE_LARGE_OFFSET,
                                                         SLAB_WIDTH + COLUMN_OFFSET - BEAM_WIDTH / 2,
                                                         COLUMN_HEIGHT - BEAM_DEPTH - GUSSET_PLATE_LARGE_DEPTH),
                                          DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    // Gusset Plate Small
    element = StructuralDomain::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetGussetPlateSmallProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0), DVec3d::From(0.0, -1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_DEPTH,
                                                         SLAB_WIDTH + COLUMN_OFFSET - BEAM_WIDTH / 2,
                                                         0.0),
                                          DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    // Gusset Plate Small
    element = StructuralDomain::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetGussetPlateSmallProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0), DVec3d::From(0.0, -1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(WALL_WIDTH - COLUMN_DEPTH - COLUMN_OFFSET,
                                                         SLAB_WIDTH + COLUMN_OFFSET - BEAM_WIDTH / 2,
                                                         0.0),
                                          DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

#pragma endregion

