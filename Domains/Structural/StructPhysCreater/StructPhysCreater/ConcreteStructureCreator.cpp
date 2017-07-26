/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/ConcreteStructureCreator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

