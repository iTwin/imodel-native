/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDbTables.h>
#include <Bentley/BeTest.h>
#include <Geom/Angle.h>

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FromGeographicTests, NullIsland)
    {
    const double tolerance = 0.001;
    GeoPoint nullIslandCarto;  nullIslandCarto.Init(0, 0, 0);
    auto ecef = EcefLocation::FromGeographic(nullIslandCarto, 0);
    EXPECT_TRUE(ecef.m_isValid);
    EXPECT_NEAR(ecef.m_origin.Distance(DPoint3d::From(6378137.0, 0.0, 0.0)), 0.0, tolerance);
    EXPECT_NEAR(ecef.m_angles.GetYaw().Degrees(), 90.00004506637939, tolerance);
    EXPECT_NEAR(ecef.m_angles.GetRoll().Degrees(), 89.99995493507596, tolerance);
    }

TEST(FromGeographicTests, WashingtonDC)
    {
    const double tolerance = 0.001;
    GeoPoint washingtonCarto;  washingtonCarto.Init(Angle::FromDegrees(-77.03652613179416).Radians(), Angle::FromDegrees(38.897712364153854).Radians(), 0); 
    auto ecef = EcefLocation::FromGeographic(washingtonCarto, 0);
    EXPECT_TRUE(ecef.m_isValid);
    EXPECT_NEAR(ecef.m_origin.Distance(DPoint3d::From(1115023.7963129042, -4843784.750164342, 3983485.836532083)), 0.0, tolerance);
    EXPECT_NEAR(ecef.m_angles.GetYaw().Degrees(), 12.963518939707493, tolerance);
    EXPECT_NEAR(ecef.m_angles.GetRoll().Degrees(), 51.10224256495991, tolerance);
    }
