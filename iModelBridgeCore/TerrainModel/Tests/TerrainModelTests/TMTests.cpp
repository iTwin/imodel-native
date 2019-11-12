/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma warning(disable:4505) // unreferenced local function has been removed [in gtest-port.h]

#include "stdafx.h"

TEST (TMTests, Clip)
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr dtm = TMHelpers::LoadTerrainModel(L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.NUnit.dll\\DTMLandXMLTest\\DTMLandXMLImportTest\\Infrasoft MX26 Demo landxml data.xml", L"TX00TRIANGLES");
    ASSERT_TRUE(dtm.IsValid());
    ASSERT_TRUE(TMHelpers::ValidateTM(*dtm, TMHelpers::ValidateParams()));

    DRange3d range;

    dtm->GetRange(range);

    double shrinkX = range.XLength() / 5;
    double shrinkY = range.YLength() / 5;

    range.low.x += shrinkX;
    range.high.x -= shrinkX;
    range.low.y += shrinkY;
    range.high.y -= shrinkY;

    DPoint3d clipRange[5];

    clipRange[0] = range.low;
    clipRange[1] = DPoint3d::From(range.low.x, range.high.x);
    clipRange[2] = range.high;
    clipRange[3] = DPoint3d::From(range.high.x, range.low.x);
    clipRange[4] = clipRange[0];

    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr clipDTMExternal = dtm->Clone();
    clipDTMExternal->Clip(clipRange, 5, DTMClipOption::External);
    ASSERT_TRUE(clipDTMExternal.IsValid());
    ASSERT_TRUE(TMHelpers::ValidateTM(*clipDTMExternal, TMHelpers::ValidateParams()));
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr clipDTMInternal = dtm->Clone();
    clipDTMInternal->Clip(clipRange, 5, DTMClipOption::Internal);
    dtm = nullptr;
    ASSERT_TRUE(clipDTMInternal.IsValid());
    ASSERT_TRUE(TMHelpers::ValidateTM(*clipDTMInternal, TMHelpers::ValidateParams()));

    SUCCEED();
    }

TEST (TMTests, Drape)
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr dtm = TMHelpers::LoadTerrainModel(L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.NUnit.dll\\DTMLandXMLTest\\DTMLandXMLImportTest\\Infrasoft MX26 Demo landxml data.xml", L"TX00TRIANGLES");
    ASSERT_TRUE(dtm.IsValid());

    DRange3d range;

    dtm->GetRange(range);

    double stepX = range.XLength() / 5;
    double stepY = range.YLength() / 5;

    DPoint3d pts[2];

    for (double x = range.low.x; x < range.high.x; x += stepX)
        {
        pts[0] = DPoint3d::From(x, range.low.y);
        pts[1] = DPoint3d::From(x, range.high.y);
        TerrainModel::DTMDrapedLinePtr ret;
        dtm->DrapeLinear(ret, pts, 2);
        ASSERT_TRUE(ret.IsValid());
        }

    for (double y = range.low.y; y < range.high.y; y += stepY)
        {
        pts[0] = DPoint3d::From(range.low.x, y);
        pts[1] = DPoint3d::From(range.high.x, y);
        TerrainModel::DTMDrapedLinePtr ret;
        dtm->DrapeLinear(ret, pts, 2);
        ASSERT_TRUE(ret.IsValid());
        }

    SUCCEED();
    }
