/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/ConcreteStructureCreator.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"


#pragma region PROTECTED_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* ConcreteStructureCreator::GetBeam12Properties()
    {
    Dgn::ColorDef transBlue(0, 0, 0xff, 0x19);
    return new PhysicalProperties(BEAM_WIDTH, BEAM_DEPTH, BEAM_12_LENGTH, ShapeTools::Shape::Rectangle, transBlue, transBlue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* ConcreteStructureCreator::GetBeam15Properties()
    {
    Dgn::ColorDef transBlue(0, 0, 0xff, 0x19);
    return new PhysicalProperties(BEAM_WIDTH, BEAM_DEPTH, BEAM_15_LENGTH, ShapeTools::Shape::Rectangle, transBlue, transBlue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* ConcreteStructureCreator::GetColumnProperties()
    {
    Dgn::ColorDef transBlue(0, 0, 0xff, 0x19);
    return new PhysicalProperties(COLUMN_WIDTH, COLUMN_DEPTH, COLUMN_HEIGHT, ShapeTools::Shape::Rectangle, transBlue, transBlue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* ConcreteStructureCreator::GetSlabProperties()
    {
    Dgn::ColorDef transRed(0xff, 0, 0, 0x19);
    return new PhysicalProperties(SLAB_WIDTH, SLAB_DEPTH, SLAB_THICKNESS, ShapeTools::Shape::Rectangle, transRed, transRed);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* ConcreteStructureCreator::GetWallProperties()
    {
    Dgn::ColorDef transRed(0xff, 0, 0, 0x19);
    return new PhysicalProperties(WALL_WIDTH, WALL_DEPTH, WALL_THICKNESS, ShapeTools::Shape::Rectangle, transRed, transRed);
    }

#pragma endregion


#pragma region PUBLIC_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus ConcreteStructureCreator::CreateSlabs(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass)
    {
    Dgn::PhysicalElementPtr element;
    PhysicalProperties* properties;
    Transform rotationMatrix;
    Transform linearMatrix;
    Dgn::DgnDbStatus status;

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetSlabProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_OFFSET, COLUMN_OFFSET, COLUMN_HEIGHT), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
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
BentleyStatus ConcreteStructureCreator::CreateWalls(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass)
    {
    Dgn::PhysicalElementPtr element;
    PhysicalProperties* properties;
    Transform rotationMatrix;
    Transform linearMatrix;
    Dgn::DgnDbStatus status;

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetWallProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0), DVec3d::From(0.0, -1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_OFFSET, WALL_THICKNESS + WALL_OFFSET, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetWallProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, 1.0), DVec3d::From(0.0, -1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_OFFSET, SLAB_WIDTH + WALL_THICKNESS + WALL_OFFSET, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

#pragma endregion

