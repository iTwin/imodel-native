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
PhysicalProperties* SteelStructureCreator::GetSlabProperties()
    {
    Dgn::ColorDef transMagenta(0xff, 0, 0xff, 0x19);
    return new PhysicalProperties(SLAB_WIDTH, SLAB_DEPTH, SLAB_THICKNESS, ShapeTools::Shape::Rectangle, transMagenta, transMagenta);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
PhysicalProperties* SteelStructureCreator::GetWallProperties()
    {
    Dgn::ColorDef transMagenta(0xff, 0, 0xff, 0x19);
    return new PhysicalProperties(WALL_WIDTH, WALL_DEPTH, WALL_THICKNESS, ShapeTools::Shape::Rectangle, transMagenta, transMagenta);
    }

#pragma endregion

