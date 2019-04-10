/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/SteelStructureCreator.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* SteelStructureCreator::GetBraceProperties()
    {
    Dgn::ColorDef transCyan(0, 0xff, 0xff, 0x19);
    return new PhysicalProperties(BRACE_LEG_A_SIZE, BRACE_LEG_B_SIZE, BRACE_LENGTH, ShapeTools::Shape::L, transCyan, transCyan);
    }

#pragma endregion


#pragma region PUBLIC_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SteelStructureCreator::CreateGussetPlates(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass)
    {
    Dgn::PhysicalElementPtr element;
    PhysicalProperties* properties;
    Transform rotationMatrix;
    Transform linearMatrix;
    Dgn::DgnDbStatus status;

    // Gusset Plate Large
    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetGussetPlateLargeProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0), DVec3d::From(0.0, -1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_OFFSET + WALL_WIDTH / 2 - GUSSET_PLATE_LARGE_OFFSET,
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
    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
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
    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
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
    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetGussetPlateLargeProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0), DVec3d::From(0.0, -1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_OFFSET + WALL_WIDTH / 2 - GUSSET_PLATE_LARGE_OFFSET,
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
    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
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
    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SteelStructureCreator::CreateBraces(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass)
    {
    Dgn::PhysicalElementPtr element;
    PhysicalProperties* properties;
    Transform rotationMatrix;
    Transform linearMatrix;
    Dgn::DgnDbStatus status;

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetBraceProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(Cos45Deg(), 0.0, -Sin45Deg()), DVec3d::From(0.0, -1.0, 0.0), DVec3d::From(-Cos45Deg(), 0.0, -Sin45Deg()));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(BRACE_LENGTH * Cos45Deg() + COLUMN_DEPTH + GUSSET_PLATE_SMALL_DEPTH * 1/3,
                                                         COLUMN_OFFSET + BEAM_WIDTH / 2 - GUSSET_PLATE_SMALL_THICKNESS,
                                                         BRACE_LENGTH * Cos45Deg() + GUSSET_PLATE_SMALL_DEPTH * 2/3),
                                          DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetBraceProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(-Cos45Deg(), 0.0, -Sin45Deg()), DVec3d::From(0.0, -1.0, 0.0), DVec3d::From(-Cos45Deg(), 0.0, Sin45Deg()));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(WALL_WIDTH - GUSSET_PLATE_SMALL_WIDTH * 1/3,
                                                         COLUMN_OFFSET + BEAM_WIDTH / 2 - GUSSET_PLATE_SMALL_THICKNESS,
                                                         GUSSET_PLATE_SMALL_DEPTH * 2/3),
                                          DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetBraceProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(Cos45Deg(), 0.0, -Sin45Deg()), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(Cos45Deg(), 0.0, Sin45Deg()));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_DEPTH + GUSSET_PLATE_SMALL_WIDTH * 1/3,
                                                         SLAB_WIDTH + COLUMN_OFFSET - BEAM_WIDTH / 2 + GUSSET_PLATE_SMALL_THICKNESS,
                                                         GUSSET_PLATE_SMALL_DEPTH * 2/3),
                                          DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetBraceProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(-Cos45Deg(), 0.0, -Sin45Deg()), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(Cos45Deg(), 0.0, -Sin45Deg()));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0 - BRACE_LENGTH * Cos45Deg() + WALL_WIDTH - GUSSET_PLATE_SMALL_WIDTH * 1/3,
                                                         SLAB_WIDTH + COLUMN_OFFSET - BEAM_WIDTH / 2 + GUSSET_PLATE_SMALL_THICKNESS,
                                                         BRACE_LENGTH * Cos45Deg() + GUSSET_PLATE_SMALL_DEPTH * 2/3),
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

