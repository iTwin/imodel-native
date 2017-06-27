/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TerrainModelTests/FormatTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma warning(disable:4505) // unreferenced local function has been removed [in gtest-port.h]

#include "stdafx.h"

TEST (TmFormats, LandXML)
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr dtm = TMHelpers::LoadTerrainModel(L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.NUnit.dll\\DTMLandXMLTest\\DTMLandXMLImportTest\\Infrasoft MX26 Demo landxml data.xml", L"TX00TRIANGLES");
    ASSERT_TRUE(dtm.IsValid());
    ASSERT_TRUE(TMHelpers::ValidateTM(*dtm, TMHelpers::ValidateParams()));

    dtm = TMHelpers::LoadTerrainModel(L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.NUnit.dll\\DTMLandXMLTest\\DTMLandXMLImportTest\\mx ground.xml");
    ASSERT_TRUE(dtm.IsValid());
    ASSERT_TRUE(TMHelpers::ValidateTM(*dtm, TMHelpers::ValidateParams()));

    dtm = TMHelpers::LoadTerrainModel(L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.NUnit.dll\\DTMLandXMLTest\\DTMLandXMLImportTest\\surf.xml");
    ASSERT_TRUE(dtm.IsValid());
    ASSERT_TRUE(TMHelpers::ValidateTM(*dtm, TMHelpers::ValidateParams()));
    SUCCEED();
    }

TEST (TmFormats, MXFil)
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr dtm = TMHelpers::LoadTerrainModel(L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.NUnit.dll\\DTMImportExportTest\\MxModelFiles\\mxModel000.fil");
    ASSERT_TRUE(dtm.IsValid());
    ASSERT_TRUE(TMHelpers::ValidateTM(*dtm, TMHelpers::ValidateParams()));
    SUCCEED();
    }

TEST (TmFormats, XYZ)
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr dtm = TMHelpers::LoadTerrainModel(L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.NUnit.dll\\DTMTriangulationTest\\DTMTriangulationXyzTest\\1M.xyz");
    ASSERT_TRUE(dtm.IsValid());
    ASSERT_TRUE(TMHelpers::ValidateTM(*dtm, TMHelpers::ValidateParams()));
    SUCCEED();
    }

TEST(TmFormats, InRoads)
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr dtm = TMHelpers::LoadTerrainModel(L"Data\\TerrainModelNet\\Bentley.Civil.Dtm.NUnit.dll\\DTMImportExportTest\\InRoadsDTMs\\exist631.dtm");
    ASSERT_TRUE(dtm.IsValid());
    ASSERT_TRUE(TMHelpers::ValidateTM(*dtm, TMHelpers::ValidateParams()));
    SUCCEED();
    }
