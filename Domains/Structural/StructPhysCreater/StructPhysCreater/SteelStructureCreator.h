/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct SteelStructureCreator : SampleStructureCreator
    {
    private:
        static const int STRUCT_ORIGIN_X = 280;
        static const int STRUCT_ORIGIN_Y = 0;
        static const int STRUCT_ORIGIN_Z = 0;

    protected:
        PhysicalProperties* SteelStructureCreator::GetBeam12Properties();
        PhysicalProperties* SteelStructureCreator::GetBeam15Properties();
        PhysicalProperties* SteelStructureCreator::GetColumnProperties();

        PhysicalProperties* SteelStructureCreator::GetGussetPlateLargeProperties();
        PhysicalProperties* SteelStructureCreator::GetGussetPlateSmallProperties();
        PhysicalProperties* SteelStructureCreator::GetBraceProperties();

    public:
        SteelStructureCreator() : SampleStructureCreator(STRUCT_ORIGIN_X, STRUCT_ORIGIN_Y, STRUCT_ORIGIN_Z) { }

        BentleyStatus SteelStructureCreator::CreateGussetPlates(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass);
        BentleyStatus SteelStructureCreator::CreateBraces(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass);
    };

