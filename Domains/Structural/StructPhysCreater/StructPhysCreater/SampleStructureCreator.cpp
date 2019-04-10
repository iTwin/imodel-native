/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/SampleStructureCreator.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"


#pragma region PROTECTED_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Placement3d SampleStructureCreator::GetStructurePlacement()
    {
    Placement3d placement;
    placement.GetOriginR() = DPoint3d::From(m_originX, m_originY, m_originZ);
    return placement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
Transform SampleStructureCreator::GetEmptyTransform()
    {
    Transform matrix;
    matrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    return matrix;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
double SampleStructureCreator::Cos45Deg()
    {
    return sqrt(2.0) / 2.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
double SampleStructureCreator::Sin45Deg()
    {
    return sqrt(2.0) / 2.0;
    }

#pragma endregion


#pragma region PUBLIC_MEMBER_FUNCTIONS

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
BentleyStatus SampleStructureCreator::CreateBeams(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass)
    {
    Dgn::PhysicalElementPtr element;
    PhysicalProperties* properties;
    Transform rotationMatrix;
    Transform linearMatrix;
    Dgn::DgnDbStatus status;

    // Beam 12'
    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetBeam12Properties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, -1.0), DVec3d::From(0.0, 1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_OFFSET, COLUMN_OFFSET, COLUMN_HEIGHT), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    // Beam 12'
    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetBeam12Properties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 0.0, -1.0), DVec3d::From(0.0, 1.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(SLAB_DEPTH - COLUMN_OFFSET, COLUMN_OFFSET, COLUMN_HEIGHT), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    // Beam 15'
    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetBeam15Properties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, -1.0), DVec3d::From(1.0, 0.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_OFFSET, COLUMN_OFFSET, COLUMN_HEIGHT), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    // Beam 15'
    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetBeam15Properties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    rotationMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, -1.0), DVec3d::From(1.0, 0.0, 0.0));
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(COLUMN_OFFSET, SLAB_WIDTH - COLUMN_OFFSET, COLUMN_HEIGHT), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
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
BentleyStatus SampleStructureCreator::CreateColumns(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass)
    {
    Dgn::PhysicalElementPtr element;
    PhysicalProperties* properties;
    Transform rotationMatrix;
    Transform linearMatrix;
    Dgn::DgnDbStatus status;

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetColumnProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    linearMatrix = GetEmptyTransform();
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetColumnProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(SLAB_DEPTH, 0.0, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetColumnProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(SLAB_DEPTH, SLAB_WIDTH, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    element = BentleyApi::Structural::StructuralDomainUtilities::CreatePhysicalElement(schema->GetName(), elementClass->GetName(), model);
    properties = GetColumnProperties();
    status = element->SetPlacement(GetStructurePlacement());
    rotationMatrix = GetEmptyTransform();
    linearMatrix = GetEmptyTransform();
    linearMatrix.InitFromOriginAndVectors(DPoint3d::From(0.0, SLAB_WIDTH, 0.0), DVec3d::From(1.0, 0.0, 0.0), DVec3d::From(0.0, 1.0, 0.0), DVec3d::From(0.0, 0.0, 1.0));
    GeometricTools::CreateStructuralMemberGeometry(element, model, schema, properties, rotationMatrix, linearMatrix);
    element->Insert(&status);
    if (Dgn::DgnDbStatus::Success != status)
        {
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

#pragma endregion

