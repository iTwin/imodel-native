// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


#include <gtest/gtest.h>
#include <Geom/GeomApi.h>
#include "BSIBaseGeomExaminerGtestHelper.h"
#include <Bentley/Bentley.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <Bentley/WString.h>
#include <TerrainModel/Drainage/Drainage.h>
#include <TerrainModel/Core/bcdtmInlines.h>
#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Formats/TerrainImporter.h>
#include <TerrainModel/Core/DTMIterators.h>

struct TMHelpers
    {
    struct ValidateParams
        {
        Bentley::TerrainModel::DTMContourParams contourParams;
        Bentley::TerrainModel::DTMFenceParams fence;

        ValidateParams()
            {
            contourParams.interval = 10;
            contourParams.depressionOption = true;
            }
        };

    static WString GetTestDataPath(WCharCP dataFile);
    static BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr LoadTerrainModel(WCharCP filename, WCharCP name = nullptr);
    static bool ValidateTM(BcDTMR dtm, const ValidateParams& params);
    };