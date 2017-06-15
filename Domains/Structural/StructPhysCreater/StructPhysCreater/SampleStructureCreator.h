/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/SampleStructureCreator.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"


//=======================================================================================
// @bsiclass                                    BentleySystems
//=======================================================================================
struct SampleStructureCreator
    {
    private:
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

        static BentleyStatus CreateBeam12Geometry(ArchitecturalPhysical::ArchitecturalBaseElementPtr element, BuildingPhysical::BuildingPhysicalModelR model);
        static BentleyStatus CreateBeam15Geometry(ArchitecturalPhysical::ArchitecturalBaseElementPtr element, BuildingPhysical::BuildingPhysicalModelR model);
        static BentleyStatus CreateColumnGeometry(ArchitecturalPhysical::ArchitecturalBaseElementPtr element, BuildingPhysical::BuildingPhysicalModelR model);
        static BentleyStatus CreateSlabGeometry(ArchitecturalPhysical::ArchitecturalBaseElementPtr element, BuildingPhysical::BuildingPhysicalModelR model);
        static BentleyStatus CreateWallGeometry(ArchitecturalPhysical::ArchitecturalBaseElementPtr element, BuildingPhysical::BuildingPhysicalModelR model);

    public:
        static BentleyStatus SampleStructureCreator::CreateBeams(BuildingPhysical::BuildingPhysicalModelR physicalModel, ECN::ECClassP elementClass);
        static BentleyStatus SampleStructureCreator::CreateColumns(BuildingPhysical::BuildingPhysicalModelR physicalModel, ECN::ECClassP elementClass);
        static BentleyStatus SampleStructureCreator::CreateSlabs(BuildingPhysical::BuildingPhysicalModelR physicalModel, ECN::ECClassP elementClass);
        static BentleyStatus SampleStructureCreator::CreateWalls(BuildingPhysical::BuildingPhysicalModelR physicalModel, ECN::ECClassP elementClass);
    };
