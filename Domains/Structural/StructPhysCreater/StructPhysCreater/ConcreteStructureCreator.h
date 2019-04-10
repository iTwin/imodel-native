/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/ConcreteStructureCreator.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct ConcreteStructureCreator : SampleStructureCreator
    {
    private:
        static const int STRUCT_ORIGIN_X = 0;
        static const int STRUCT_ORIGIN_Y = 0;
        static const int STRUCT_ORIGIN_Z = 0;

    protected:
        PhysicalProperties* ConcreteStructureCreator::GetBeam12Properties();
        PhysicalProperties* ConcreteStructureCreator::GetBeam15Properties();
        PhysicalProperties* ConcreteStructureCreator::GetColumnProperties();

        PhysicalProperties* ConcreteStructureCreator::GetSlabProperties();
        PhysicalProperties* ConcreteStructureCreator::GetWallProperties();

    public:
        ConcreteStructureCreator() : SampleStructureCreator(STRUCT_ORIGIN_X, STRUCT_ORIGIN_Y, STRUCT_ORIGIN_Z) { }

        BentleyStatus ConcreteStructureCreator::CreateSlabs(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass);
        BentleyStatus ConcreteStructureCreator::CreateWalls(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass);
    };

