/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/SampleStructureCreator.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"
#include "GeometricTools.h"


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct SampleStructureCreator
    {
    private:
        static const int DISTANCE_BETWEEN_STRUCTURES = 280;

        // Beam dimensions: 10" x 14" x 12' & 10" x 14" x 15'
        static const int BEAM_WIDTH = 10;
        static const int BEAM_DEPTH = 14;
        static const int BEAM_12_LENGTH = 144;
        static const int BEAM_15_LENGTH = 180;

        // Column dimensions: 12" x 12" x 10'
        static const int COLUMN_WIDTH = 12;
        static const int COLUMN_DEPTH = 12;
        static const int COLUMN_HEIGHT = 120;

        // Slab dimensions: 12' x 15' x 9"
        static const int SLAB_WIDTH = 144;
        static const int SLAB_DEPTH = 180;
        static const int SLAB_THICKNESS = 9;

        // Wall dimensions: 15' x 10' x 8"
        static const int WALL_WIDTH = 180;
        static const int WALL_DEPTH = 120;
        static const int WALL_THICKNESS = 8;

        // Other placement computations
        static const int COLUMN_OFFSET = COLUMN_WIDTH / 2;
        static const int WALL_OFFSET = (COLUMN_DEPTH - WALL_THICKNESS) / 2;

        static double STRUCT_ORIGIN_X;
        static double STRUCT_ORIGIN_Y;
        static double STRUCT_ORIGIN_Z;

        static Dgn::Placement3d SampleStructureCreator::GetStructurePlacement();

        static PhysicalProperties* SampleStructureCreator::GetBeam12Properties();
        static PhysicalProperties* SampleStructureCreator::GetBeam15Properties();
        static PhysicalProperties* SampleStructureCreator::GetColumnProperties();
        static PhysicalProperties* SampleStructureCreator::GetSlabProperties();
        static PhysicalProperties* SampleStructureCreator::GetWallProperties();

        static Transform SampleStructureCreator::GetEmptyTransform();

    public:
        static void InitConcreteStructurePlacement()
            {
            STRUCT_ORIGIN_X = 0.0;
            STRUCT_ORIGIN_Y = 0.0;
            STRUCT_ORIGIN_Z = 0.0;
            }

        static void InitSteelStructurePlacement()
            {
            STRUCT_ORIGIN_X = DISTANCE_BETWEEN_STRUCTURES;
            STRUCT_ORIGIN_Y = 0.0;
            STRUCT_ORIGIN_Z = 0.0;
            }

        static BentleyStatus SampleStructureCreator::CreateBeams(StructuralPhysical::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass);
        static BentleyStatus SampleStructureCreator::CreateColumns(StructuralPhysical::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass);
        static BentleyStatus SampleStructureCreator::CreateSlabs(StructuralPhysical::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass);
        static BentleyStatus SampleStructureCreator::CreateWalls(StructuralPhysical::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass);
    };
