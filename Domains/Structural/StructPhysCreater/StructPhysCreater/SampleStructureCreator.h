/*--------------------------------------------------------------------------------------+
|
|     $Source: StructPhysCreater/StructPhysCreater/SampleStructureCreator.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
        double m_originX;
        double m_originY;
        double m_originZ;

    protected:
        // Beam dimensions: 10" x 14" x 12' & 10" x 14" x 18'
        static const int BEAM_WIDTH = 10;
        static const int BEAM_DEPTH = 14;
        static const int BEAM_12_LENGTH = 144;
        static const int BEAM_15_LENGTH = 216;

        // Column dimensions: 12" x 12" x 10'
        static const int COLUMN_WIDTH = 12;
        static const int COLUMN_DEPTH = 12;
        static const int COLUMN_HEIGHT = 120;

        // Slab dimensions: 12' x 18' x 9"
        static const int SLAB_WIDTH = 144;
        static const int SLAB_DEPTH = 216;
        static const int SLAB_THICKNESS = 9;

        // Wall dimensions: 18' x 10' x 8"
        static const int WALL_WIDTH = 216;
        static const int WALL_DEPTH = 120;
        static const int WALL_THICKNESS = 8;

        // Gusset Plate (large) dimensions: 3' x 1.5' x 1"
        static const int GUSSET_PLATE_LARGE_WIDTH = 36;
        static const int GUSSET_PLATE_LARGE_DEPTH = 18;
        static const int GUSSET_PLATE_LARGE_THICKNESS = 1;

        // Gusset Plate (small) dimensions: 1.5' x 1.5' x 1"
        static const int GUSSET_PLATE_SMALL_WIDTH = 18;
        static const int GUSSET_PLATE_SMALL_DEPTH = 18;
        static const int GUSSET_PLATE_SMALL_THICKNESS = 1;

        // Brace: 7" x 4" x 3/8" (10' length)
        static const int BRACE_LEG_A_SIZE = 7;
        static const int BRACE_LEG_B_SIZE = 4;
        static const int BRACE_LENGTH = 120;

        // Other placement computations
        static const int COLUMN_OFFSET = COLUMN_WIDTH / 2;
        static const int WALL_OFFSET = (COLUMN_DEPTH - WALL_THICKNESS) / 2;

        static const int GUSSET_PLATE_LARGE_OFFSET = GUSSET_PLATE_LARGE_WIDTH / 2;


        SampleStructureCreator(double originX, double originY, double originZ) :
            m_originX(originX), m_originY(originY), m_originZ(originZ) { }

        Placement3d SampleStructureCreator::GetStructurePlacement();

        virtual PhysicalProperties* SampleStructureCreator::GetBeam12Properties() = 0;
        virtual PhysicalProperties* SampleStructureCreator::GetBeam15Properties() = 0;
        virtual PhysicalProperties* SampleStructureCreator::GetColumnProperties() = 0;

        static Transform SampleStructureCreator::GetEmptyTransform();

        static double SampleStructureCreator::Cos45Deg();
        static double SampleStructureCreator::Sin45Deg();

    public:
        BentleyStatus SampleStructureCreator::CreateBeams(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass);
        BentleyStatus SampleStructureCreator::CreateColumns(BentleyApi::Structural::StructuralPhysicalModelR model, ECN::ECSchemaCP schema, ECN::ECClassP elementClass);
    };

