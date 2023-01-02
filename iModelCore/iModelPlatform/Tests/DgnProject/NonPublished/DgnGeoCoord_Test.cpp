/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/DgnGeoCoord.h>
#include <Bentley/Desktop/FileSystem.h>

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool InitGeoCoord()
    {
    if (!GeoCoordinates::BaseGCS::IsLibraryInitialized())
        {
        BeFileName moduleFileName = Desktop::FileSystem::GetExecutableDir();

        BeFileName path(BeFileName::GetDirectoryName(moduleFileName).c_str());

        path.AppendToPath(L"Assets\\DgnGeoCoord");

        GeoCoordinates::BaseGCS::Initialize(path.GetNameUtf8().c_str());
        path.AppendToPath(L"allEarth.itwin-workspace");
        GeoCoordinates::BaseGCS::AddWorkspaceDb(path.GetNameUtf8(), nullptr, 0);
        }

    return GeoCoordinates::BaseGCS::IsLibraryInitialized();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class BaseDgnGeoCoordTest : public DgnDbTestFixture
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseDgnGeoCoordTest, GeneralTestsOneDb)
    {
    Utf8String errorMessage;

    ASSERT_TRUE(InitGeoCoord());

    DgnDbPtr dgnProj;

    BeFileName dgndbFileName;
    BeTest::GetHost().GetOutputRoot(dgndbFileName);
    dgndbFileName.AppendToPath(L"MyTestFile.ibim");

    if (BeFileName::DoesPathExist(dgndbFileName))
        BeFileName::BeDeleteFile(dgndbFileName);

    DbResult status;
    CreateDgnDbParams params("Test");
    dgnProj = DgnDb::CreateIModel(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()), params);

    GeoCoordinates::BaseGCSPtr theNewGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-10N");

    ASSERT_TRUE(theNewGCS.IsValid() && theNewGCS->IsValid());

    auto& geolocation = dgnProj->GeoLocation();

    geolocation.SetGCS(theNewGCS.get());

    geolocation.Save();

    dgnProj->SaveChanges();
    dgnProj->CloseDb();

    DgnDbPtr reOpenedProj = DgnDb::OpenIModelDb(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()), DgnDb::OpenParams(Db::OpenMode::Readonly));

    DgnGeoLocation& storedGeoLocation = reOpenedProj->GeoLocation();

    DgnGCS* theGCS = storedGeoLocation.GetDgnGCS();

    //---------------------------------------------
    DgnGCS* sameGCS = DgnGCS::FromProject(*reOpenedProj);

    EXPECT_EQ(theGCS, sameGCS); // Should be same pointer on reload

    //---------------------------------------------
    EXPECT_EQ(theGCS->GetPaperScale(), 1.0);

    //---------------------------------------------
    Utf8String displayName;
    EXPECT_STREQ(theGCS->GetDisplayName(displayName), "UTM84-10N");

    //---------------------------------------------
    GeoCoordinates::BaseGCSPtr firstGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-11N");

    ASSERT_TRUE(firstGCS.IsValid() && firstGCS->IsValid());

    Transform tfReproject;

    // Define NULL extent
    DRange3d extent;
    extent.low.x = 0.0;
    extent.low.y = 0.0;
    extent.low.z = 0.0;

    extent.high.x = 0.0;
    extent.high.y = 0.0;
    extent.high.z = 0.0;

    double maxError;
    double meanError;

    // Null extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == theGCS->GetLinearTransformToBaseGCS(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    //---------------------------------------------
    extent.high.x = 0.001; // x dim extent too small
    extent.high.y = 0.01;
    extent.high.z = 0.01;

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == theGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    //---------------------------------------------
    extent.high.x = 0.01;
    extent.high.y = 0.001; // y extent too small

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == theGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    //---------------------------------------------
    extent.high.y = 0.01;
    extent.high.z = 0.001; // z extent too small

    // too small extent will result in an error
    ASSERT_FALSE(REPROJECT_Success == theGCS->GetLinearTransform(&tfReproject, extent, *firstGCS, &maxError, &meanError));

    //---------------------------------------------
    GeoCoordinates::BaseGCSPtr secondGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-11N");

    ASSERT_TRUE(secondGCS.IsValid() && secondGCS->IsValid());

    extent.low.x = 1011760.9129999126;
    extent.low.y = 7056097.8990061926;
    extent.low.z = 350.0;
    extent.high.x = 1011934.5129998885;
    extent.high.y = 7056793.8990063900;
    extent.high.z = 990.0;

    ASSERT_TRUE(REPROJECT_Success == theGCS->GetLinearTransform(&tfReproject, extent, *secondGCS, &maxError, &meanError));

    ASSERT_TRUE(maxError < 2.0);
    ASSERT_TRUE(meanError < 0.5);

    //---------------------------------------------
    GeoPoint2d inLatLong;
    DPoint2d outUors;
    inLatLong.latitude = 32.714938;
    inLatLong.longitude = -117.1612;

    EXPECT_TRUE(REPROJECT_Success == theGCS->UorsFromLatLong2D(outUors, inLatLong));
    EXPECT_NEAR(outUors.x, 1047580.0068159207, 0.001);
    EXPECT_NEAR(outUors.y, 3634796.9022512850, 0.001);

    //---------------------------------------------
    GeoPoint2d outLatLong;

    EXPECT_TRUE(REPROJECT_Success == theGCS->LatLongFromUors2D(outLatLong, outUors));

    EXPECT_NEAR(outLatLong.latitude, inLatLong.latitude, 0.0000001);
    EXPECT_NEAR(outLatLong.longitude, inLatLong.longitude, 0.0000001);

    //---------------------------------------------
    GeoPoint inLatLong3d;
    DPoint3d outUors3d;
    inLatLong3d.latitude = 32.714938;
    inLatLong3d.longitude = -117.1612;
    inLatLong3d.elevation = 10.23;

    EXPECT_TRUE(REPROJECT_Success == theGCS->UorsFromLatLong(outUors3d, inLatLong3d));
    EXPECT_NEAR(outUors3d.x, 1047580.0068159207, 0.001);
    EXPECT_NEAR(outUors3d.y, 3634796.9022512850, 0.001);
    EXPECT_NEAR(outUors3d.z, inLatLong3d.elevation, 0.0000001);
    EXPECT_NEAR(outUors.x, outUors3d.x, 0.00000001);
    EXPECT_NEAR(outUors.y, outUors3d.y, 0.00000001);

    //---------------------------------------------
    GeoPoint outLatLong3d;

    EXPECT_TRUE(REPROJECT_Success == theGCS->LatLongFromUors(outLatLong3d, outUors3d));

    EXPECT_NEAR(outLatLong3d.latitude, inLatLong3d.latitude, 0.0000001);
    EXPECT_NEAR(outLatLong3d.longitude, inLatLong3d.longitude, 0.0000001);
    EXPECT_NEAR(outLatLong3d.elevation, inLatLong3d.elevation, 0.0000001);

    EXPECT_NEAR(outLatLong3d.latitude, inLatLong.latitude, 0.0000001);
    EXPECT_NEAR(outLatLong3d.longitude, inLatLong.longitude, 0.0000001);
    }

    /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseDgnGeoCoordTest, GeneralTestsTwoDb)
    {
    Utf8String errorMessage;

    ASSERT_TRUE(InitGeoCoord());

    //---------------------------------------------
    DgnDbPtr dgnProj1;

    BeFileName dgndbFileName1;
    BeTest::GetHost().GetOutputRoot(dgndbFileName1);
    dgndbFileName1.AppendToPath(L"MyTestFile1.ibim");

    if (BeFileName::DoesPathExist(dgndbFileName1))
        BeFileName::BeDeleteFile(dgndbFileName1);

    DbResult status;
    CreateDgnDbParams params1("Test1");
    dgnProj1 = DgnDb::CreateIModel(&status, BeFileName(dgndbFileName1.GetNameUtf8().c_str()), params1);

    GeoCoordinates::BaseGCSPtr theNewGCS1 = GeoCoordinates::BaseGCS::CreateGCS("UTM84-10N");

    ASSERT_TRUE(theNewGCS1.IsValid() && theNewGCS1->IsValid());

    auto& geolocation1 = dgnProj1->GeoLocation();

    geolocation1.SetGCS(theNewGCS1.get());

    geolocation1.Save();

    dgnProj1->SaveChanges();
    dgnProj1->CloseDb();

        //---------------------------------------------
    DgnDbPtr dgnProj2;

    BeFileName dgndbFileName2;
    BeTest::GetHost().GetOutputRoot(dgndbFileName2);
    dgndbFileName2.AppendToPath(L"MyTestFile2.ibim");

    if (BeFileName::DoesPathExist(dgndbFileName2))
        BeFileName::BeDeleteFile(dgndbFileName2);

    CreateDgnDbParams params2("Test2");
    dgnProj2 = DgnDb::CreateIModel(&status, BeFileName(dgndbFileName2.GetNameUtf8().c_str()), params2);

    GeoCoordinates::BaseGCSPtr theNewGCS2 = GeoCoordinates::BaseGCS::CreateGCS("UTM84-11N");

    ASSERT_TRUE(theNewGCS2.IsValid() && theNewGCS2->IsValid());

    auto& geolocation2 = dgnProj2->GeoLocation();

    geolocation2.SetGCS(theNewGCS2.get());

    geolocation2.Save();

    dgnProj2->SaveChanges();
    dgnProj2->CloseDb();

    //---------------------------------------------

    DgnDbPtr reOpenedProj1 = DgnDb::OpenIModelDb(&status, BeFileName(dgndbFileName1.GetNameUtf8().c_str()), DgnDb::OpenParams(Db::OpenMode::Readonly));

    DgnGeoLocation& storedGeoLocation1 = reOpenedProj1->GeoLocation();

    DgnGCS* theGCS1 = storedGeoLocation1.GetDgnGCS();


    DgnDbPtr reOpenedProj2 = DgnDb::OpenIModelDb(&status, BeFileName(dgndbFileName2.GetNameUtf8().c_str()), DgnDb::OpenParams(Db::OpenMode::Readonly));

    DgnGeoLocation& storedGeoLocation2 = reOpenedProj2->GeoLocation();

    DgnGCS* theGCS2 = storedGeoLocation2.GetDgnGCS();

    //---------------------------------------------
    EXPECT_TRUE(theGCS1->UnitsIdentical(*theGCS2));

    //---------------------------------------------
    Transform tfReproject;

    DPoint3d extent;
    DPoint3d elementOrigin;
    elementOrigin.x = 1011760.9129999126;
    elementOrigin.y = 7056097.8990061926;
    elementOrigin.z = 350.0;

    double maxError = 0.0;;
    double meanError = 0.0;
    extent.x = 10.5129998885;
    extent.y = 20.8990063900;
    extent.z = 990.0;

    EXPECT_TRUE(REPROJECT_Success == theGCS1->GetLocalTransform(&tfReproject, elementOrigin, &extent, true, true, GeoCoordInterpretation::Cartesian, *theGCS2));

    EXPECT_TRUE(!tfReproject.IsIdentity());
    ASSERT_TRUE(maxError < 2.0);
    ASSERT_TRUE(meanError < 0.5);


    //---------------------------------------------
    DPoint2d outUorDest;
    GeoPoint2d outLatLongDest;
    GeoPoint2d outLatLongSrc;
    DPoint2d inUor;

    inUor.x = 1047580.0068159207;
    inUor.y = 3634796.9022512850;

    EXPECT_TRUE(REPROJECT_Success ==  theGCS1->ReprojectUors2D(&outUorDest, &outLatLongDest, &outLatLongSrc, &inUor, 1, *theGCS2));

    EXPECT_NEAR(outLatLongSrc.latitude, 32.714938, 0.0000001);
    EXPECT_NEAR(outLatLongSrc.longitude, -117.1612, 0.0000001);

    EXPECT_NEAR(outLatLongDest.latitude, 32.714938, 0.0000001);
    EXPECT_NEAR(outLatLongDest.longitude, -117.1612, 0.0000001);

    EXPECT_NEAR(outUorDest.x, 484893.12168253760, 0.0000001);
    EXPECT_NEAR(outUorDest.y, 3619697.1492840736, 0.0000001);

    //---------------------------------------------
    DPoint3d outUorDest3d;
    GeoPoint outLatLongDest3d;
    GeoPoint outLatLongSrc3d;
    DPoint3d inUor3d;

    inUor3d.x = 1047580.0068159207;
    inUor3d.y = 3634796.9022512850;
    inUor3d.z = 350.0;

    EXPECT_TRUE(REPROJECT_Success ==  theGCS1->ReprojectUors(&outUorDest3d, &outLatLongDest3d, &outLatLongSrc3d, &inUor3d, 1, *theGCS2));

    EXPECT_NEAR(outLatLongSrc3d.latitude, 32.714938, 0.0000001);
    EXPECT_NEAR(outLatLongSrc3d.longitude, -117.1612, 0.0000001);
    EXPECT_NEAR(outLatLongSrc3d.elevation, 350.0, 0.0000001);

    EXPECT_NEAR(outLatLongDest3d.latitude, 32.714938, 0.0000001);
    EXPECT_NEAR(outLatLongDest3d.longitude, -117.1612, 0.0000001);
    EXPECT_NEAR(outLatLongDest3d.elevation, 350.0, 0.0000001);

    EXPECT_NEAR(outUorDest3d.x, 484893.12168253760, 0.0000001);
    EXPECT_NEAR(outUorDest3d.y, 3619697.1492840736, 0.0000001);
    EXPECT_NEAR(outUorDest3d.z, 350.0, 0.0000001);

    //---------------------------------------------
    DPoint2d outUorDestMat[4];
    GeoPoint2d outLatLongDestMat[4];
    GeoPoint2d outLatLongSrcMat[4];
    DPoint2d inUorMat[4];

    inUorMat[0].x = 1047580.0068159207;
    inUorMat[0].y = 3634796.9022512850;

    inUorMat[1].x = 1047591.0068159207;
    inUorMat[1].y = 3634784.8022512850;

    inUorMat[2].x = 1047571.0068159207;
    inUorMat[2].y = 3634774.8022512850;

    inUorMat[3].x = 1047574.5068159207;
    inUorMat[3].y = 3634769.7022512850;

    EXPECT_TRUE(REPROJECT_Success ==  theGCS1->ReprojectUors2D(outUorDestMat, outLatLongDestMat, outLatLongSrcMat, inUorMat, 4, *theGCS2));

    EXPECT_NEAR(outLatLongSrcMat[0].latitude, 32.714938, 0.0000001);
    EXPECT_NEAR(outLatLongSrcMat[0].longitude, -117.1612, 0.0000001);

    EXPECT_NEAR(outLatLongDestMat[0].latitude, 32.714938, 0.0000001);
    EXPECT_NEAR(outLatLongDestMat[0].longitude, -117.1612, 0.0000001);

    EXPECT_NEAR(outUorDestMat[0].x, 484893.12168253760, 0.0000001);
    EXPECT_NEAR(outUorDestMat[0].y, 3619697.1492840736, 0.0000001);

    EXPECT_NEAR(outLatLongSrcMat[3].latitude, 32.714696628919121, 0.0000001);
    EXPECT_NEAR(outLatLongSrcMat[3].longitude, -117.16127433906790, 0.0000001);

    EXPECT_NEAR(outLatLongDestMat[3].latitude, 32.714696628919121, 0.0000001);
    EXPECT_NEAR(outLatLongDestMat[3].longitude, -117.16127433906790, 0.0000001);

    EXPECT_NEAR(outUorDestMat[3].x, 484886.11411265371, 0.0000001);
    EXPECT_NEAR(outUorDestMat[3].y, 3619670.4038204621, 0.0000001);

    //---------------------------------------------
    DPoint3d outUorDest3dMat[4];
    GeoPoint outLatLongDest3dMat[4];
    GeoPoint outLatLongSrc3dMat[4];
    DPoint3d inUor3dMat[4];

    inUor3d.x = 1047580.0068159207;
    inUor3d.y = 3634796.9022512850;
    inUor3d.z = 350.0;

    inUor3dMat[0].x = 1047580.0068159207;
    inUor3dMat[0].y = 3634796.9022512850;
    inUor3dMat[0].z = 363.902;

    inUor3dMat[1].x = 1047591.0068159207;
    inUor3dMat[1].y = 3634784.8022512850;
    inUor3dMat[1].z = 362.82;

    inUor3dMat[2].x = 1047571.0068159207;
    inUor3dMat[2].y = 3634774.8022512850;
    inUor3dMat[2].z = 163.102;

    inUor3dMat[3].x = 1047574.5068159207;
    inUor3dMat[3].y = 3634769.7022512850;
    inUor3dMat[3].z = 261.502;

    EXPECT_TRUE(REPROJECT_Success ==  theGCS1->ReprojectUors(outUorDest3dMat, outLatLongDest3dMat, outLatLongSrc3dMat, inUor3dMat, 4, *theGCS2));

    EXPECT_NEAR(outLatLongSrc3dMat[0].latitude, 32.714938, 0.0000001);
    EXPECT_NEAR(outLatLongSrc3dMat[0].longitude, -117.1612, 0.0000001);
    EXPECT_NEAR(outLatLongSrc3dMat[0].elevation, 363.902, 0.0000001);

    EXPECT_NEAR(outLatLongDest3dMat[0].latitude, 32.714938, 0.0000001);
    EXPECT_NEAR(outLatLongDest3dMat[0].longitude, -117.1612, 0.0000001);
    EXPECT_NEAR(outLatLongDest3dMat[0].elevation, 363.902, 0.0000001);

    EXPECT_NEAR(outUorDest3dMat[0].x, 484893.12168253760, 0.0000001);
    EXPECT_NEAR(outUorDest3dMat[0].y, 3619697.1492840736, 0.0000001);
    EXPECT_NEAR(outUorDest3dMat[0].z, 363.902, 0.0000001);

    // No need testing intermediate points

    EXPECT_NEAR(outLatLongSrc3dMat[3].latitude, 32.714696628919121, 0.0000001);
    EXPECT_NEAR(outLatLongSrc3dMat[3].longitude, -117.16127433906790, 0.0000001);
    EXPECT_NEAR(outLatLongSrc3dMat[3].elevation, 261.502, 0.0000001);

    EXPECT_NEAR(outLatLongDest3dMat[3].latitude, 32.714696628919121, 0.0000001);
    EXPECT_NEAR(outLatLongDest3dMat[3].longitude, -117.16127433906790, 0.0000001);
    EXPECT_NEAR(outLatLongDest3dMat[3].elevation, 261.502, 0.0000001);

    EXPECT_NEAR(outUorDest3dMat[3].x, 484886.11411265371, 0.0000001);
    EXPECT_NEAR(outUorDest3dMat[3].y, 3619670.4038204621, 0.0000001);
    EXPECT_NEAR(outUorDest3dMat[3].z, 261.502, 0.0000001);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class BaseECEFLocationTests : public DgnDbTestFixture
    {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseECEFLocationTests, ECEFLocationIsInvalidWithoutGCS)
    {
    WString errorMessage;

    ASSERT_TRUE(InitGeoCoord());

    SetupSeedProject();

    EXPECT_FALSE(m_db->GeoLocation().GetEcefLocation().m_isValid);
}

struct EcefLocationTestData
    {
    // Setup data
    WString description;
    Utf8String gcsCode;
    DPoint3d origin;

    // Ecef location data
    DPoint3d ecefOrigin;
    double yaw;
    double pitch;
    double roll;
    DPoint3d xVector;
    DPoint3d yVector;

    // Linear projection data
    Transform dbToEcef;
    DPoint3d ecefOriginProjected;
    double ecefElevation;

    // Tolerances for comparisons
    double positionTolerance;
    double ecefNearTolerance;
    double ecefFarTolerance;
    double ecefVeryFarTolerance;
    };

#if 1
EcefLocationTestData ecefLocationTestData[] = {
        {
        L"",
        "UTM84-10N",
        {525179.58, 4649776.22, 1113.0},
        {-4701293.9485125579, -6320797.9838896338, 789540.41246992536},
        {-32.832084943108939},
        {-0.15119091233930054},
        {48.000209611989945},
        {0.84058987395837903, -0.54238966898992658, -0.0026398114860057831},
        {0.36457771807909012, 0.56140045681968331, 0.74343626108020544},
        {0.84058987395837903, 0.36457771807909012, -0.40143508633631564, -4701293.9485125579,
         -0.54238966898992658, 0.56140045681968331, -0.62539656146327050, -6320797.9838896338,
         -0.0026398114860057831, 0.74343626108020544, 0.66912555799944784, 789540.41246992536},
         {-2564701.9206129969, -3995552.6334454701, 4246315.3356936639},
         1113.0000002011657,
         5.e-3,
         1.e-3,
         1.2,
         5.7e1
        },
        {
        L"Lambert conic, Montana",
        "MT83/2011",
        {405991.628, 264405.235, 1215},
        {-2091112.4372350213, -4107209.3719788459, 4420673.5772885066},
        {-20.687295155753521},
        {1.2738285634471911},
        {43.383358237590329},
        {0.93601174908690155, -0.35345219867303967, 0.022247797809541225},
        {0.24264744529500604, 0.68583620339632034, 0.68723615445196629},
        {0.93601586972363293, 0.24264566623605788, -0.25775973153953391, -2091112.4372350213,
         -0.35344717185944319, 0.68583197426050901, -0.63688980599754874, -4107209.3719788459,
         0.022241326048970222, 0.68724382948130369, 0.72658880793270708, 4420673.5772885066},
         {-1647254.2241230316, -4070142.2214834802, 4612297.0410979493},
         1215,
         5.e-3,
         2.e-3,
         1.6e-0,
         7.8e+1
        },
        {
        L"Near equator, low distortion, WGS84 World Mercator",
        "EPSG:3395",
        {-8751072.679, -18796.871, 4032.0},
        {9843535.7036586441, -4523630.3364648549, 11.851752660259081},
        {11.387785583778546},
        {0.0},
        {90.169979356396638},
        {0.98092871555127203, 0.19757231511175632, 0.00000000000000000},
        {0.00058613927103579044, -0.0029101287946105003, 1.0006276436361077},
        {0.98092871555127203, 0.00058613927103579044, 0.19744749103508713, 9843535.7036586441,
         0.19757231511175632, -0.0029101287946105003, -0.98030897527476768, -4523630.3364648549,
         0.00000000000000000, 1.0006276436361077, -0.0029666950777395510, 11.851752660259081},
         {1260142.3116509318, -6256497.9298388865, -18808.778698355072},
         4032.0,
         5.e-3,
         1.e-2,
         1.3e+0,
         6.2e+2
        },
        {
        L"Northern region, high distortion, WGS84 World Mercator",
        "EPSG:3395",
        {-7926041.535, 5886717.295, 8.0},
        {7496905.8920612894, -5172216.1156419171, 1872346.9924495001},
        {18.799166448450375},
        {0.0},
        {43.148261468983833},
        {0.64856322994455695, 0.22077840426936746, 0.00000000000000000},
        {-0.16107689356431365, 0.47318295249715447, 0.46853969432413578},
        {0.64856322994455695, -0.16107689356431365, 0.22038440451633756, 7496905.8920612894,
         0.22077840426936746, 0.47318295249715447, -0.64740580807955128, -5172216.1156419171,
         0.00000000000000000, 0.46853969432413578, 0.72958648144743088, 1872346.9924495001},
         {1408154.4213522915, -4136625.8269942468, 4630513.5511132553},
         8.0,
         5.e-3,
         5.e-1,
         1.e+3,
         5.e+5
        },
        {
        L"Southern region, Australia",
        "MGA2020-54",
        {280407.849, 6133690.198, 25},
        {-1015292.1714535691, 1465267.4172233185, -8651862.3947606832},
        {-130.61583357974490},
        {-1.1285774816020400},
        {124.92324674609067},
        {-0.65073381597176194, -0.75879968842491508, -0.019692369271069765},
        {-0.44499175343662500, 0.36034705303609371, 0.81960455281659961},
        {-0.65073381597176194, -0.44499175343662500, -0.61505376208818452, -1015292.1714535691,
         -0.75879968842491508, 0.36034705303609371, 0.54231381583680882, 1465267.4172233185,
         -0.019692369271069765, 0.81960455281659961, -0.57236753480057767, -8651862.3947606832},
         {-3927219.9736508843, 3462764.7737012878, -3630198.1870107166},
         25,
         5.e-3,
         1.e-3,
         4.e-1,
         2.e+1
        }
    };
#else
    EcefLocationTestData ecefLocationTestData[] = {
        {
        L"Melbourne Australia",
        L"VictoriaGrid",
        {2498417.353,4410667.407,0.0},
        {-1015292.1714535691, 1465267.4172233185, -8651862.3947606832},
        {-130.61583357974490},
        {-1.1285774816020400},
        {124.92324674609067},
        {-0.65073381597176194, -0.75879968842491508, -0.019692369271069765},
        {-0.44499175343662500, 0.36034705303609371, 0.81960455281659961},
        {-0.65073381597176194, -0.44499175343662500, -0.61505376208818452, -1015292.1714535691,
         -0.75879968842491508, 0.36034705303609371, 0.54231381583680882, 1465267.4172233185,
         -0.019692369271069765, 0.81960455281659961, -0.57236753480057767, -8651862.3947606832},
         {-3927219.9736508843, 3462764.7737012878, -3630198.1870107166},
         25,
         1.e-3,
         4.e-1,
         2.e+1
        }
        };
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
    TEST_F(BaseECEFLocationTests, GeoreferencedDbReturnsValidECEFLocation)
        {
        // Initialize the project
        WString errorMessage;

        ASSERT_TRUE(InitGeoCoord());

        for (const auto& baseline : ecefLocationTestData)
            {

            SetupSeedProject();

            GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS(baseline.gcsCode.c_str());

            ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());

            theGCS->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);

            // Set GCS to the project
            {
            m_db->GeoLocation().SetGCS(theGCS.get());
            m_db->GeoLocation().Save();
            m_db->SaveChanges();
            }

            // Add geometry to the project
            DPoint3d dbOrigin = baseline.origin;
            AngleInDegrees zeroDegrees = AngleInDegrees::FromDegrees(0);
            Placement3d placement(dbOrigin, YawPitchRollAngles(zeroDegrees, zeroDegrees, zeroDegrees));
            {
            auto model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "ECEFLocationTest");

            DgnCategoryId cat = DgnDbTestUtils::GetFirstSpatialCategoryId(*m_db);
            DgnClassId classId(m_db->Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject));
            auto elem = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(*m_db, model->GetModelId(), classId, cat, placement));

            auto builder = GeometryBuilder::Create(*elem->ToGeometrySource());
            builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1, 2, 0))));
            builder->Finish(*elem->ToGeometrySourceP());

            DgnElementId elemId;
            auto newElem = m_db->Elements().Insert(*elem);
            if (newElem.IsValid())
                elemId = newElem->GetElementId();
            }

            // Project should have valid ECEFLocation
            auto& geolocation = m_db->GeoLocation();
            const auto& ecefLocation = geolocation.GetEcefLocation();
            EXPECT_TRUE(ecefLocation.m_isValid);

            // Check ECEFLocation origin
            EXPECT_NEAR(ecefLocation.m_origin.x, baseline.ecefOrigin.x, baseline.positionTolerance);
            EXPECT_NEAR(ecefLocation.m_origin.y, baseline.ecefOrigin.y, baseline.positionTolerance);
            EXPECT_NEAR(ecefLocation.m_origin.z, baseline.ecefOrigin.z, baseline.positionTolerance);

            // Verify ECEFLocation angles
            EXPECT_NEAR(ecefLocation.m_angles.GetYaw().Degrees(),  baseline.yaw , baseline.positionTolerance);
            EXPECT_NEAR(ecefLocation.m_angles.GetPitch().Degrees(),baseline.pitch , baseline.positionTolerance);
            EXPECT_NEAR(ecefLocation.m_angles.GetRoll().Degrees(), baseline.roll , baseline.positionTolerance);

            // Check vectors
            EXPECT_TRUE(ecefLocation.m_haveVectors);
            EXPECT_NEAR(ecefLocation.m_xVector.x, baseline.xVector.x, baseline.positionTolerance);
            EXPECT_NEAR(ecefLocation.m_xVector.y, baseline.xVector.y, baseline.positionTolerance);
            EXPECT_NEAR(ecefLocation.m_xVector.z, baseline.xVector.z, baseline.positionTolerance);
            EXPECT_NEAR(ecefLocation.m_yVector.x, baseline.yVector.x, baseline.positionTolerance);
            EXPECT_NEAR(ecefLocation.m_yVector.y, baseline.yVector.y, baseline.positionTolerance);
            EXPECT_NEAR(ecefLocation.m_yVector.z, baseline.yVector.z, baseline.positionTolerance);

            // Project extents should be computed correctly (DB contains a segment (0,0,0) -> (1,2,0) starting from dbOrigin)
            auto extents = geolocation.ComputeProjectExtents();
            EXPECT_NEAR(extents.low.x, dbOrigin.x, baseline.positionTolerance);
            EXPECT_NEAR(extents.low.y, dbOrigin.y, baseline.positionTolerance);
            EXPECT_NEAR(extents.low.z, dbOrigin.z, baseline.positionTolerance);
            EXPECT_NEAR(extents.high.x, dbOrigin.x + 1.0, baseline.positionTolerance);
            EXPECT_NEAR(extents.high.y, dbOrigin.y + 2.0, baseline.positionTolerance);
            EXPECT_NEAR(extents.high.z, dbOrigin.z, baseline.positionTolerance);

            // Check cartographic origin
            EXPECT_TRUE(ecefLocation.m_haveCartographicOrigin);

            GeoPoint cartographicOrigin;
            EXPECT_TRUE(SUCCESS == geolocation.LatLongFromXyz(cartographicOrigin, extents.GetCenter()));

            EXPECT_NEAR(cartographicOrigin.latitude, ecefLocation.m_cartographicOrigin.latitude, baseline.positionTolerance);
            EXPECT_NEAR(cartographicOrigin.longitude, ecefLocation.m_cartographicOrigin.longitude, baseline.positionTolerance);

            // Transform at project center should be computed correctly
            Transform dbToECEF;
            geolocation.GetEcefTransformAtPoint(dbToECEF, extents.GetCenter());

            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    EXPECT_NEAR(dbToECEF.form3d[i][j], baseline.dbToEcef.form3d[i][j], baseline.positionTolerance);
            for (int i = 0; i < 3; i++)
                EXPECT_NEAR(dbToECEF.form3d[i][3], baseline.dbToEcef.form3d[i][3], baseline.positionTolerance);

            // Check reprojected origin using transform
            DPoint3d ecefOrigin;
            dbToECEF.Multiply(ecefOrigin, dbOrigin);
            EXPECT_NEAR(ecefOrigin.x, baseline.ecefOriginProjected.x, baseline.positionTolerance);
            EXPECT_NEAR(ecefOrigin.y, baseline.ecefOriginProjected.y, baseline.positionTolerance);
            EXPECT_NEAR(ecefOrigin.z, baseline.ecefOriginProjected.z, baseline.positionTolerance);

            // Consistent lat-long values from DB and ECEF origins
            GeoPoint outLatLong3d, outLatLong3dXYZ;
            auto wgs84GCS = DgnGCS::CreateGCS("LL84", *m_db);
            wgs84GCS->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);

            EXPECT_TRUE(REPROJECT_Success == wgs84GCS->LatLongFromUorsXYZ(outLatLong3dXYZ, ecefOrigin));

            DgnGCS* theDGNGCS = geolocation.GetDgnGCS();
            EXPECT_TRUE(REPROJECT_Success == theDGNGCS->LatLongFromUors(outLatLong3d, dbOrigin));
            EXPECT_TRUE(REPROJECT_Success == theDGNGCS->LatLongFromLatLong(outLatLong3d, outLatLong3d, *wgs84GCS));

            EXPECT_NEAR(outLatLong3dXYZ.latitude, outLatLong3d.latitude,   baseline.positionTolerance);
            EXPECT_NEAR(outLatLong3dXYZ.longitude, outLatLong3d.longitude, baseline.positionTolerance);
            EXPECT_NEAR(outLatLong3dXYZ.elevation, baseline.ecefElevation, baseline.positionTolerance);

            auto testPositionAndDistanceHelper = [&] (DPoint3dCR arbitraryDBPoint, const double& positionTolerance, const double& distanceTolerance) -> double
                {
                DPoint3d arbitraryEcefPoint;
                dbToECEF.Multiply(arbitraryEcefPoint, arbitraryDBPoint);

                EXPECT_TRUE(REPROJECT_Success == theDGNGCS->LatLongFromUors(outLatLong3d, arbitraryDBPoint));
                EXPECT_TRUE(REPROJECT_Success == theDGNGCS->LatLongFromLatLong(outLatLong3d, outLatLong3d, *wgs84GCS));
                DPoint3d trueECEFPoint;
                EXPECT_TRUE(REPROJECT_Success == wgs84GCS->XYZFromLatLong(trueECEFPoint, outLatLong3d));
                double error = trueECEFPoint.Distance(arbitraryEcefPoint);
                EXPECT_TRUE(error < positionTolerance);

                EXPECT_NEAR(arbitraryDBPoint.Distance(dbOrigin), arbitraryEcefPoint.Distance(ecefOrigin), distanceTolerance);
                return error;
                };

            // Computed transform should reproject (approximately) DB coordinates to ECEF coordinates:
            // At project center, approximation should be tight
            DPoint3d arbitraryDBPoint;
            double previousError;
            {
            arbitraryDBPoint = extents.GetCenter();
            double error = testPositionAndDistanceHelper(arbitraryDBPoint, baseline.positionTolerance, baseline.ecefNearTolerance);
            EXPECT_TRUE(error < 1.e-3 * baseline.positionTolerance);
            previousError = error;
            }
            // Near project center, approximation should be tight
            {
            //=====
            arbitraryDBPoint = DPoint3d::From(1.0, 1.0, 0.0);
            placement.GetTransform().Multiply(arbitraryDBPoint, arbitraryDBPoint);
            double error = testPositionAndDistanceHelper(arbitraryDBPoint, baseline.positionTolerance, baseline.ecefNearTolerance);
            EXPECT_TRUE(error > previousError);
            double newError = std::max(previousError, error);

            //=====
            arbitraryDBPoint = DPoint3d::From(1.0, 0.0, 0.0);
            placement.GetTransform().Multiply(arbitraryDBPoint, arbitraryDBPoint);
            error = testPositionAndDistanceHelper(arbitraryDBPoint, baseline.positionTolerance, baseline.ecefNearTolerance);
            EXPECT_TRUE(error > previousError);
            newError = std::max(previousError, error);

            //=====
            arbitraryDBPoint = DPoint3d::From(0.0, 1.0, 0.0);
            placement.GetTransform().Multiply(arbitraryDBPoint, arbitraryDBPoint);
            error = testPositionAndDistanceHelper(arbitraryDBPoint, baseline.positionTolerance, baseline.ecefNearTolerance);
            EXPECT_TRUE(error > previousError);
            newError = std::max(previousError, error);

            previousError = newError;
            }
            // Far from project center, approximation should be less accurate
            {
            arbitraryDBPoint = DPoint3d::From(2000.0, 0.0, 0.0);
            placement.GetTransform().Multiply(arbitraryDBPoint, arbitraryDBPoint);
            double error = testPositionAndDistanceHelper(arbitraryDBPoint, 1.e+3 * baseline.positionTolerance, baseline.ecefFarTolerance);
            EXPECT_TRUE(error > previousError);
            double newError = std::max(previousError, error);

            arbitraryDBPoint = DPoint3d::From(0.0, 2000.0, 0.0);
            placement.GetTransform().Multiply(arbitraryDBPoint, arbitraryDBPoint);
            error = testPositionAndDistanceHelper(arbitraryDBPoint, 1.e+3 * baseline.positionTolerance, baseline.ecefFarTolerance);
            EXPECT_TRUE(error > previousError);
            newError = std::max(previousError, error);

            previousError = newError;
            }
            // Very far from project center, approximation should be alot less accurate
            {
            arbitraryDBPoint = DPoint3d::From(100000.0, 0.0, 0.0);
            placement.GetTransform().Multiply(arbitraryDBPoint, arbitraryDBPoint);
            double error = testPositionAndDistanceHelper(arbitraryDBPoint, 1.e+6 * baseline.positionTolerance, baseline.ecefVeryFarTolerance);
            EXPECT_TRUE(error > previousError);

            arbitraryDBPoint = DPoint3d::From(0.0, 100000.0, 0.0);
            placement.GetTransform().Multiply(arbitraryDBPoint, arbitraryDBPoint);
            error = testPositionAndDistanceHelper(arbitraryDBPoint, 1.e+6 * baseline.positionTolerance, baseline.ecefVeryFarTolerance);
            EXPECT_TRUE(error > previousError);
            }
            }
        }

struct LocationTestData
    {
    // Setup data
    WString description;
    Utf8String gcsCode;
    DPoint3d origin;

    double originTolerance;
    double testPointToleranceNonRigid; // yields to more accurate results
    double testPointToleranceNormalizedAndRigid;
    };

LocationTestData locations[] = {
        {
        L"GCS with strong distortions, origin somewhere in Salt Lake City",
        "EPSG:3857",
        {-12460190.505, 4972163.139, 1289.287},
        0.001,
        0.2,
        340.0
        },
        {
        L"Normally used GCS, origin somewhere in Salt Lake City",
        "UTM84-12N",
        {421310.659, 4508873.947, 1289.296},
        0.001,
        0.2,
        0.7
        }
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseECEFLocationTests, EffectOfNormalizationAndOrthogonalization)
    {
    ASSERT_TRUE(InitGeoCoord());

    for (const auto& baseline : locations)
        {
        GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS(baseline.gcsCode.c_str());
        theGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);

        ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());

        auto modelOrigin = baseline.origin;
        auto modelTestPoint = DPoint3d::From(modelOrigin.x + 1000.0, modelOrigin.y + 1000.0, baseline.origin.z);

        GeoPoint modelOriginLatLong;
        theGCS->LatLongFromCartesian(modelOriginLatLong, modelOrigin);

        // Compute ground truth ECEF points
        DPoint3d ecefOrigin, ecefTestPoint;
        theGCS->ECEFFromCartesian(ecefOrigin, modelOrigin);
        theGCS->ECEFFromCartesian(ecefTestPoint, modelTestPoint);

        // Compute model coordinates to ECEF transforms...
        // ... non rigid transform
        EcefLocation ecefLocation(modelOriginLatLong, modelOrigin, &*theGCS);
        DVec3d zVector = DVec3d::FromNormalizedCrossProduct(ecefLocation.m_xVector, ecefLocation.m_yVector);
        auto rMatrix = RotMatrix::FromColumnVectors(ecefLocation.m_xVector, ecefLocation.m_yVector, zVector);
        auto ecefTransNonRigid = Transform::From(rMatrix, ecefLocation.m_origin);

        auto rawEcefTransform = ecefTransNonRigid;
        rawEcefTransform.TranslateInLocalCoordinates(rawEcefTransform, modelOrigin.x, modelOrigin.y, modelOrigin.z);


        // ... rigid transform
        auto rigidMatrix = rMatrix;
        rigidMatrix.SquareAndNormalizeColumns(rigidMatrix, 0, 1);
        auto ecefTransRigid = rawEcefTransform;
        ecefTransRigid.SetMatrix(rigidMatrix);
        ecefTransRigid.TranslateInLocalCoordinates(ecefTransRigid, -modelOrigin.x, -modelOrigin.y, -modelOrigin.z);

        // ... normalized transform
        auto xVectorNorm = ecefLocation.m_xVector;
        xVectorNorm.Normalize();
        auto yVectorNorm = ecefLocation.m_yVector;
        yVectorNorm.Normalize();
        zVector = DVec3d::FromNormalizedCrossProduct(xVectorNorm, yVectorNorm);
        auto normalizedMatrix = RotMatrix::FromColumnVectors(xVectorNorm, yVectorNorm, zVector);
        auto ecefTransNormalized = rawEcefTransform;
        ecefTransNormalized.SetMatrix(normalizedMatrix);
        ecefTransNormalized.TranslateInLocalCoordinates(ecefTransNormalized, -modelOrigin.x, -modelOrigin.y, -modelOrigin.z);

        // Compute ECEF origin from computed linear transforms (no error)
        DPoint3d ecefOriginNonRigid, ecefOriginRigid, ecefOriginNormalized;
        ecefTransNonRigid.Multiply(ecefOriginNonRigid, modelOrigin);
        ecefTransRigid.Multiply(ecefOriginRigid, modelOrigin);
        ecefTransNormalized.Multiply(ecefOriginNormalized, modelOrigin);

        EXPECT_NEAR(ecefOriginNonRigid.Distance(ecefOrigin),   0.0, baseline.originTolerance);
        EXPECT_NEAR(ecefOriginRigid.Distance(ecefOrigin),      0.0, baseline.originTolerance);
        EXPECT_NEAR(ecefOriginNormalized.Distance(ecefOrigin), 0.0, baseline.originTolerance);

        // Compute ECEF test point from computed linear transforms (errors due to earth curvature, distortions, etc...)
        DPoint3d ecefTestPointNonRigid, ecefTestPointRigid, ecefTestPointNormalized;
        ecefTransNonRigid.Multiply(ecefTestPointNonRigid, modelTestPoint);
        ecefTransRigid.Multiply(ecefTestPointRigid, modelTestPoint);
        ecefTransNormalized.Multiply(ecefTestPointNormalized, modelTestPoint);

        EXPECT_LT(ecefTestPointNonRigid.Distance(ecefTestPoint), baseline.testPointToleranceNonRigid);
        EXPECT_GT(ecefTestPointRigid.Distance(ecefTestPoint), baseline.testPointToleranceNormalizedAndRigid);
        EXPECT_GT(ecefTestPointNormalized.Distance(ecefTestPoint), baseline.testPointToleranceNormalizedAndRigid);

        // Compare transforms relative to baseline ECEF test point (non rigid is more accurate)
        auto nonRigidDistance = ecefTestPointNonRigid.Distance(ecefTestPoint);
        auto rigidDistance = ecefTestPointRigid.Distance(ecefTestPoint);
        auto normalizedDistance = ecefTestPointNormalized.Distance(ecefTestPoint);

        EXPECT_TRUE(nonRigidDistance < rigidDistance);
        EXPECT_TRUE(nonRigidDistance < normalizedDistance);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseECEFLocationTests, EffectOfTangentVSSecantPlanes)
    {

    // This test compares a few ways to compute ECEF location starting from a specified GCS and an
    // origin point within this GCS. The following details are adapted from the text found here:
    // https://www.geometrictools.com/Documentation/ConvertingBetweenCoordinateSystems.pdf
    // The tests below all begin by computing an affine transformation from GCS coordinates to ECEF
    // coordinates by first computing a list of linearly independent directions from the selected model
    // origin which forms a basis for R^3. Then any point p in R^3 can be written as p = A*x where the columns of A
    // are the 3 linearly independent directions and x is a vector reprensenting the coordinates of the point
    // in the local frame. If we repeat this process to compute a local frame in ECEF, then any point y in R^3 can
    // be expressed as y = A_ECEF * x (the local frame is being scaled, rotated and translated by using the same
    // coefficient vector x). Combining this with p = A*x we get y = A_ECEF * A^(-1) * p. The matrix
    // A_ECEF * A^-1 reprensents a linear transform form local GCS coordinates to ECEF coordinates.

    ASSERT_TRUE(InitGeoCoord());

    for (const auto& baseline : locations)
        {
        GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS(baseline.gcsCode.c_str());
        theGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);

        ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());

        Transform tangent1, tangent2, secant;
        auto modelOrigin = baseline.origin;
        { // secant plane (PowerPlatform computes frame based on root node extent which could be large)
        auto eastPoint = DPoint3d::From(modelOrigin.x + 5000.0, modelOrigin.y, baseline.origin.z);
        auto northPoint = DPoint3d::From(modelOrigin.x, modelOrigin.y + 5000.0, baseline.origin.z);
        auto elevationPoint = DPoint3d::From(modelOrigin.x, modelOrigin.y, baseline.origin.z + 5000.0);

        DPoint3d ecefOrigin, ecefX, ecefY, ecefZ;
        theGCS->ECEFFromCartesian(ecefOrigin, modelOrigin);
        theGCS->ECEFFromCartesian(ecefX, eastPoint);
        theGCS->ECEFFromCartesian(ecefY, northPoint);
        theGCS->ECEFFromCartesian(ecefZ, elevationPoint);

        Transform frameA, frameAInverse;
        frameA.InitFrom4Points(modelOrigin, eastPoint, northPoint, elevationPoint);
        frameAInverse.InverseOf(frameA);

        secant.InitFrom4Points(ecefOrigin, ecefX, ecefY, ecefZ);

        secant = secant.FromProduct(secant, frameAInverse);
        }
        { // tangent plane version 1 (PowerPlatform's formula)
        auto eastPoint = DPoint3d::From(modelOrigin.x + 1.0, modelOrigin.y, baseline.origin.z);
        auto northPoint = DPoint3d::From(modelOrigin.x, modelOrigin.y + 1.0, baseline.origin.z);
        auto elevationPoint = DPoint3d::From(modelOrigin.x, modelOrigin.y, baseline.origin.z + 1.0);

        DPoint3d ecefOrigin, ecefX, ecefY, ecefZ;
        theGCS->ECEFFromCartesian(ecefOrigin, modelOrigin);
        theGCS->ECEFFromCartesian(ecefX, eastPoint);
        theGCS->ECEFFromCartesian(ecefY, northPoint);
        theGCS->ECEFFromCartesian(ecefZ, elevationPoint);

        Transform frameA, frameAInverse;
        frameA.InitFrom4Points(modelOrigin, eastPoint, northPoint, elevationPoint);
        frameAInverse.InverseOf(frameA);

        tangent1.InitFrom4Points(ecefOrigin, ecefX, ecefY, ecefZ);

        tangent1 = tangent1.FromProduct(tangent1, frameAInverse);
        }
        { // tangent plane version 2 (connector's formula, equivalent to version 1, optimised to avoid matrix inversion)
        auto eastPoint = DPoint3d::From(modelOrigin.x + 1.0, modelOrigin.y, baseline.origin.z);
        auto northPoint = DPoint3d::From(modelOrigin.x, modelOrigin.y + 1.0, baseline.origin.z);
        auto elevationPoint = DPoint3d::From(modelOrigin.x, modelOrigin.y, baseline.origin.z + 1.0);

        DPoint3d ecefOrigin, ecefX, ecefY;
        theGCS->ECEFFromCartesian(ecefOrigin, modelOrigin);
        theGCS->ECEFFromCartesian(ecefX, eastPoint);
        theGCS->ECEFFromCartesian(ecefY, northPoint);

        DVec3d xVector, yVector, zVector;
        xVector.DifferenceOf(ecefX, ecefOrigin);
        yVector.DifferenceOf(ecefY, ecefOrigin);
        zVector = DVec3d::FromNormalizedCrossProduct(xVector, yVector);
        RotMatrix rMatrix = RotMatrix::FromColumnVectors(xVector, yVector, zVector);

        tangent2 = Transform::From(rMatrix, ecefOrigin);
        tangent2.TranslateInLocalCoordinates(tangent2, -modelOrigin.x, -modelOrigin.y, -modelOrigin.z);
        }

        // Compare values at distant point. Tangent plane should be more precise close to the
        // chosen origin point (first part), up to about half way through the distance chosen to compute
        // the secant plane in the positive direction, where the earth surface begins to curve back
        // towards the secant plane (second part).
        {
        auto modelTestPoint = DPoint3d::From(modelOrigin.x + 1000.0, modelOrigin.y + 1000.0, baseline.origin.z);
        DPoint3d secantTestPoint, tangent1TestPoint, tangent2TestPoint, groundTruthTestPoint;
        secant.Multiply(secantTestPoint, modelTestPoint);
        tangent1.Multiply(tangent1TestPoint, modelTestPoint);
        tangent2.Multiply(tangent2TestPoint, modelTestPoint);
        theGCS->ECEFFromCartesian(groundTruthTestPoint, modelTestPoint);

        auto secantError = secantTestPoint.Distance(groundTruthTestPoint);
        auto tangentError1 = tangent1TestPoint.Distance(groundTruthTestPoint);
        auto tangentError2 = tangent2TestPoint.Distance(groundTruthTestPoint);

        EXPECT_TRUE(tangentError1 < secantError);
        EXPECT_TRUE(tangentError2 < secantError);
        EXPECT_NEAR(tangentError1, tangentError2, 0.0001);
        }
        {
        auto modelTestPoint = DPoint3d::From(modelOrigin.x + 3000.0, modelOrigin.y + 3000.0, baseline.origin.z);
        DPoint3d secantTestPoint, tangent1TestPoint, tangent2TestPoint, groundTruthTestPoint;
        secant.Multiply(secantTestPoint, modelTestPoint);
        tangent1.Multiply(tangent1TestPoint, modelTestPoint);
        tangent2.Multiply(tangent2TestPoint, modelTestPoint);
        theGCS->ECEFFromCartesian(groundTruthTestPoint, modelTestPoint);

        auto secantError = secantTestPoint.Distance(groundTruthTestPoint);
        auto tangentError1 = tangent1TestPoint.Distance(groundTruthTestPoint);
        auto tangentError2 = tangent2TestPoint.Distance(groundTruthTestPoint);

        EXPECT_TRUE(tangentError1 > secantError);
        EXPECT_TRUE(tangentError2 > secantError);
        EXPECT_NEAR(tangentError1, tangentError2, 0.0001);
        }
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseECEFLocationTests, EffectOfElevation)
    {
    ASSERT_TRUE(InitGeoCoord());

    for (const auto& baseline : locations)
        {
        GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS(baseline.gcsCode.c_str());
        theGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);

        ASSERT_TRUE(theGCS.IsValid() && theGCS->IsValid());

        // t: tangent plane at model origin point
        // t_g: tangent plane using ground elevation ( z = 0 )
        Transform t, t_g;
        auto modelOrigin = baseline.origin;

        { // Compute tangent plane at model origin point
        auto eastPoint =        DPoint3d::From(modelOrigin.x + 1.0, modelOrigin.y,  modelOrigin.z);
        auto northPoint =       DPoint3d::From(modelOrigin.x, modelOrigin.y + 1.0,  modelOrigin.z);
        auto elevationPoint =   DPoint3d::From(modelOrigin.x, modelOrigin.y,        modelOrigin.z + 1.0);

        DPoint3d ecefOrigin, ecefX, ecefY, ecefZ;
        theGCS->ECEFFromCartesian(ecefOrigin, modelOrigin);
        theGCS->ECEFFromCartesian(ecefX, eastPoint);
        theGCS->ECEFFromCartesian(ecefY, northPoint);
        theGCS->ECEFFromCartesian(ecefZ, elevationPoint);

        Transform frameA, frameAInverse;
        frameA.InitFrom4Points(modelOrigin, eastPoint, northPoint, elevationPoint);
        frameAInverse.InverseOf(frameA);

        t.InitFrom4Points(ecefOrigin, ecefX, ecefY, ecefZ);

        t = t.FromProduct(t, frameAInverse);
        }

        { // Compute tangent plane using ground elevation ( z = 0 )
        auto origin = modelOrigin;
        origin.z = 0.0;

        auto eastPoint = DPoint3d::From(origin.x + 1.0, origin.y, origin.z);
        auto northPoint = DPoint3d::From(origin.x, origin.y + 1.0, origin.z);
        auto elevationPoint = DPoint3d::From(origin.x, origin.y, origin.z + 1.0);

        DPoint3d ecefOrigin, ecefX, ecefY, ecefZ;
        theGCS->ECEFFromCartesian(ecefOrigin, origin);
        theGCS->ECEFFromCartesian(ecefX, eastPoint);
        theGCS->ECEFFromCartesian(ecefY, northPoint);
        theGCS->ECEFFromCartesian(ecefZ, elevationPoint);

        Transform frameA, frameAInverse;
        frameA.InitFrom4Points(origin, eastPoint, northPoint, elevationPoint);
        frameAInverse.InverseOf(frameA);

        t_g.InitFrom4Points(ecefOrigin, ecefX, ecefY, ecefZ);

        t_g = t_g.FromProduct(t_g, frameAInverse);
        }

        // Transforms should reproject the model origin at the ECEF model origin
        DPoint3d modelOrigin_ecef, modelOrigin_ecef_g, groundTruthModelOrigin_ecef;
        t.Multiply(modelOrigin_ecef, modelOrigin);
        t_g.Multiply(modelOrigin_ecef_g, modelOrigin);
        theGCS->ECEFFromCartesian(groundTruthModelOrigin_ecef, modelOrigin);

        EXPECT_NEAR(modelOrigin_ecef.Distance(modelOrigin_ecef_g), 0.0, 0.0001);
        EXPECT_NEAR(modelOrigin_ecef.Distance(groundTruthModelOrigin_ecef), 0.0, 0.0001);

        // Test transforms at distant point...
        // ... setup data
        auto testPoint = DPoint3d::From(modelOrigin.x + 1000.0, modelOrigin.y + 1000.0, modelOrigin.z);
        auto groundTestPoint = DPoint3d::From(testPoint.x, testPoint.y, 0.0);
        DPoint3d testPoint_ecef, testPoint_ecef_g, groundTruthTestPoint_ecef;
        t.Multiply(testPoint_ecef, testPoint);
        t_g.Multiply(testPoint_ecef_g, testPoint);
        theGCS->ECEFFromCartesian(groundTruthTestPoint_ecef, testPoint);

        // ... Transform t yields more precise results than t_g with respect to the ellipsoid passing through the model origin
        auto error_t = testPoint_ecef.Distance(groundTruthTestPoint_ecef);
        auto error_t_g = testPoint_ecef_g.Distance(groundTruthTestPoint_ecef);
        EXPECT_TRUE(error_t < error_t_g);

        // ... Elevation vector is perpendicular to both tangent planes
        DPoint3d groundTestPoint_ecef_g;
        t_g.Multiply(groundTestPoint_ecef_g, groundTestPoint);

        DVec3d elevation_v;
        elevation_v.DifferenceOf(testPoint_ecef_g, groundTestPoint_ecef_g);
        {
        auto tangent_direction = t_g.ColumnX();
        auto dot = tangent_direction.DotProduct(elevation_v) / ( tangent_direction.Magnitude() * elevation_v.Magnitude() );
        EXPECT_NEAR(dot, 0.0, 0.0001);
        }
        {
        auto tangent_direction = t_g.ColumnY();
        auto dot = tangent_direction.DotProduct(elevation_v) / ( tangent_direction.Magnitude() * elevation_v.Magnitude() );
        EXPECT_NEAR(dot, 0.0, 0.0001);
        }

        // ... Because of the previous fact, t_g maps to points that are closer than those mapped by t
        EXPECT_TRUE(testPoint_ecef.Distance(modelOrigin_ecef) > testPoint_ecef_g.Distance(modelOrigin_ecef));

        // t and t_g have near parallel axes...
        { // ...X
        auto tangent_direction = t.ColumnX();
        auto tangent_direction_g = t_g.ColumnX();
        auto dot = tangent_direction.DotProduct(tangent_direction_g) / ( tangent_direction.Magnitude() * tangent_direction_g.Magnitude() );
        EXPECT_NEAR(dot, 1.0, 1.e-6);
        }
        { // ...Y
        auto tangent_direction = t.ColumnY();
        auto tangent_direction_g = t_g.ColumnY();
        auto dot = tangent_direction.DotProduct(tangent_direction_g) / ( tangent_direction.Magnitude() * tangent_direction_g.Magnitude() );
        EXPECT_NEAR(dot, 1.0, 1.e-6);
        }
        { // ...Z
        auto tangent_direction = t.ColumnZ();
        auto tangent_direction_g = t_g.ColumnZ();
        auto dot = tangent_direction.DotProduct(tangent_direction_g) / ( tangent_direction.Magnitude() * tangent_direction_g.Magnitude() );
        EXPECT_NEAR(dot, 1.0, 1.e-6);
        }

        // The projection on the tangent plane is simply the extension of the line passing through the
        // center of the earth and the expected ECEF point all the way to the intersection point on the
        // tangent plane t(P). Therefore the test point vector is parallel to the expected test point vector.
        auto testPointAngle = testPoint_ecef.DotProduct(groundTruthTestPoint_ecef) / ( testPoint_ecef.Magnitude() * groundTruthTestPoint_ecef.Magnitude());
        EXPECT_NEAR(testPointAngle, 1.0, 1.e-6);

        // Tangent plane based on the model origin height will be more precise relative to the model origin height ellipsoid as long as the
        // ratio of the error computed here is less than 0.5. This can be shown by using the Pythagorean theorem on the right angle triangle formed
        // by t(P), t_g(P) and t_g((Px, Py, 0).
        auto deltaTestPointECEFGroundTruthECEF = testPoint_ecef.Distance(groundTruthTestPoint_ecef);
        auto deltaTestPointGroundECEFGroundTruthECEF = groundTestPoint_ecef_g.Distance(groundTruthTestPoint_ecef);
        auto ratio1 = deltaTestPointECEFGroundTruthECEF / ( deltaTestPointECEFGroundTruthECEF + deltaTestPointGroundECEFGroundTruthECEF );
        auto ratio2 = deltaTestPointGroundECEFGroundTruthECEF / ( deltaTestPointECEFGroundTruthECEF + deltaTestPointGroundECEFGroundTruthECEF );

        EXPECT_TRUE(ratio1 < 0.5 && ratio1 < ratio2);
        }

    }

Utf8String testValuesCorrect[] = {
R"X({"horizontalCRS" : {"id" : "UTM84-10N"}, "verticalCRS" : {"id" : "GEOID"}})X",
R"X({"horizontalCRS" : {"id" : "UTM83-10"}, "verticalCRS" : {"id" : "NAVD88"}})X",
R"X({"horizontalCRS" : {"id" : "UTM84-10N"}, "verticalCRS" : {"id" : "GEOID"}})X",
R"X({"horizontalCRS" : {"epsg" : 27700} , "verticalCRS" : {"id" : "GEOID"}})X",
R"X({"horizontalCRS" : {"epsg" : 3857}, "verticalCRS" : {"id" : "ELLIPSOID"}})X",
R"X({"horizontalCRS" : {"id" : "GRMNY-S5"}, "verticalCRS" : {"id" : "LOCAL_ELLIPSOID"}})X"
R"X({
   "additionalTransform" : {
      "helmert2DWithZOffset" : {
         "rotDeg" : 90.0,
         "scale" : 1.0,
         "translationX" : 0.0,
         "translationY" : -20.0,
         "translationZ" : 300.0
      }
   },
   "horizontalCRS" : {
      "id" : "UTM84-10N",
   },
   "verticalCRS" : {
      "id" : "GEOID"
   })X",
R"X({ "horizontalCRS": {
    "id": "TESTGCS7",
        "description" : "USES CUSTOM DATUM (TEST-GRID)",
        "source" : "EPSG:2165",
        "deprecated" : false,
        "epsg" : 0,
        "datumId" : "TEST-GRID",
        "datum" : {
        "id": "TEST-GRID",
            "description" : "TEST GRID - Uses custom ell but custom transfo",
            "deprecated" : false,
            "source" : "Emmo",
            "epsg" : 0,
            "ellipsoidId" : "CustomEllipsoid1",
            "ellipsoid" : {
            "id": "CustomEllipsoid1",
                "description" : "Custom Ellipsoid1 Description",
                "source" : "Custom Ellipsoid1 Source",
                "equatorialRadius" : 6378171.0,
                "polarRadius" : 6356795.719195306
            },
            "transforms": [
                {
                "method": "GridFiles",
                    "sourceEllipsoidId" : "CustomEllipsoid1",
                    "sourceEllipsoid" : {
                    "id": "CustomEllipsoid1",
                        "equatorialRadius" : 6378171.0,
                        "polarRadius" : 6356795.719195306
                    },
                    "targetEllipsoidId" : "WGS84",
                    "targetEllipsoid" : {
                        "id": "WGS84"
                    },
                    "gridFile" : {
                            "files": [
                            {
                                "fileName": "./Australia/Agd66/A66National(13.09.01).gsb",
                                    "format" : "NTv2",
                                    "direction" : "Direct"
                            }
                            ]
                        }
            }
            ]
    },
        "unit": "Meter",
        "projection" : {
        "method": "TransverseMercator",
            "centralMeridian" : 143,
            "latitudeOfOrigin" : 0,
            "scaleFactor" : 0.9996,
            "falseEasting" : 500000.0,
            "falseNorthing" : 10000000.0
    },
    "extent": {
      "southWest": {
        "latitude": 3.9,
        "longitude": -7.55
      },
      "northEast": {
        "latitude": 5.13,
        "longitude": -2.75
      }
    }
},
   "verticalCRS" : {
      "id" : "GEOID"
   }} )X"
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class SetAndGetDgnGeoCoord : public DgnDbTestFixture
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SetAndGetDgnGeoCoord, SetAndGetAndCompare)
    {

    for (Utf8String theJson : testValuesCorrect)
        {
        Utf8String errorMessage;

        ASSERT_TRUE(InitGeoCoord());

        DgnDbPtr dgnProj;

        BeFileName dgndbFileName;
        BeTest::GetHost().GetOutputRoot(dgndbFileName);
        dgndbFileName.AppendToPath(L"MyFile.ibim");

        if (BeFileName::DoesPathExist(dgndbFileName))
            BeFileName::BeDeleteFile(dgndbFileName);

        DbResult status;
        CreateDgnDbParams params("Test");
        dgnProj = DgnDb::CreateIModel(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()), params);

        GeoCoordinates::BaseGCSPtr theNewGCS = GeoCoordinates::BaseGCS::CreateGCS();

        ASSERT_TRUE(theNewGCS.IsValid());

        ASSERT_TRUE(SUCCESS == theNewGCS->FromJson(Json::Value::From(theJson), errorMessage));

        ASSERT_TRUE(theNewGCS->IsValid());

        auto& geolocation = dgnProj->GeoLocation();

        geolocation.SetGCS(theNewGCS.get());

        geolocation.Save();

        dgnProj->SaveChanges();
        dgnProj->CloseDb();

        DgnDbPtr reOpenedProj = DgnDb::OpenIModelDb(&status, BeFileName(dgndbFileName.GetNameUtf8().c_str()), DgnDb::OpenParams(Db::OpenMode::Readonly));

        DgnGeoLocation& storedGeoLocation = reOpenedProj->GeoLocation();

        DgnGCS* theGCS = storedGeoLocation.GetDgnGCS();

        ASSERT_TRUE(theGCS->IsEquivalent(*theNewGCS));
        }

    }

// INSTANTIATE_TEST_SUITE_P (DgnGeoCoordTest,
//                         SetAndGetDgnGeoCoord,
//                         ::testing::ValuesIn (testValuesCorrect)
//                         );



// The following, although perfectly valid cannot be stored in a DGN file for diverse reasons such as
// More than one transform to WGS84 or grid file name more than 75 characters long.
// We will want those to be storable to an iModel though but this will require changes to the way
// the GCS is stored... upcoming later.
Utf8String testValuesNotStorableYet[] = {
R"X({ "horizontalCRS": {
    "id": "TESTGCS9",
    "description" : "USES CUSTOM DATUM (TESTDATUM3)",
    "source" : "EPSG:2165",
    "deprecated" : false,
    "epsg" : 0,
    "datumId" : "TEST-GRID",
    "datum" : {
        "id": "TEST-GRID",
        "description" : "TEST DATUM 3 - Uses custom ell but custom transfo",
        "deprecated" : false,
        "source" : "Emmo",
        "epsg" : 0,
        "ellipsoidId" : "CustomEllipsoid1",
        "ellipsoid" : {
            "id": "CustomEllipsoid1",
            "description" : "Custom Ellipsoid1 Description",
            "source" : "Custom Ellipsoid1 Source",
            "equatorialRadius" : 6378171.0,
            "polarRadius" : 6356795.719195306
        },
        "transforms": [
        {
            "method": "GridFiles",
            "sourceEllipsoidId" : "CustomEllipsoid1",
            "sourceEllipsoid" : {
                "id": "CustomEllipsoid1",
                "equatorialRadius" : 6378171.0,
                "polarRadius" : 6356795.719195306
            },
            "targetEllipsoidId" : "CustomEllipsoid2",
            "targetEllipsoid" : {
                "id": "CustomEllipsoid2",
                "equatorialRadius" : 6378171.1,
                "polarRadius" : 6356795.8
            },
            "gridFile" : {
                "files": [
                    {
                       "fileName": "./Australia/Agd66/A66National(13.09.01).gsb",
                       "format" : "NTv2",
                       "direction" : "Direct"
                    }
                ]
            }
        },
        {
            "method": "Geocentric",
            "sourceEllipsoidId" : "CustomEllipsoid2",
            "sourceEllipsoid" : {
                "id": "CustomEllipsoid2",
                "equatorialRadius" : 6378171.1,
                "polarRadius" : 6356795.8
            },
            "targetEllipsoidId" : "CustomEllipsoid3",
            "targetEllipsoid" : {
                "id": "CustomEllipsoid3",
                "equatorialRadius" : 6378174.1,
                "polarRadius" : 6356796.1
            },
            "geocentric" : {
                "delta": {
                    "x": -115,
                    "y" : 118,
                    "z" : 426
                }
            }
        },
        {
            "method": "PositionalVector",
            "geocentric" : null,
            "positionalVector" : {
                "scalePPM": 2.4985,
                "delta" : {
                    "x": -120.271,
                    "y" : -64.543,
                    "z" : 161.632
                },
                "rotation" : {
                    "x": 0.2175,
                    "y" : -0.0672,
                    "z" : -0.1291
                }
            },
            "sourceEllipsoidId": "CustomEllipsoid3",
            "sourceEllipsoid" : {
                "id": "CustomEllipsoid3",
                "equatorialRadius" : 6378174.1,
                "polarRadius" : 6356796.1
            },
            "targetEllipsoidId" : "WGS84",
            "targetEllipsoid" : {
                "id": "WGS84",
                "equatorialRadius" : 6378137.0,
                "polarRadius" : 6356752.3142
            }
        }
        ]
    },
    "unit": "Meter",
    "projection" : {
    "method": "TransverseMercator",
        "centralMeridian" : 143,
        "latitudeOfOrigin" : 0,
        "scaleFactor" : 0.9996,
        "falseEasting" : 500000.0,
        "falseNorthing" : 10000000.0
    },
    "extent": {
        "southWest": {
            "latitude": 3.9,
            "longitude": -7.55
        },
        "northEast": {
            "latitude": 5.13,
            "longitude": -2.75
        }
    }
},
"verticalCRS" : {
      "id" : "GEOID"
}} )X",
R"X({ "horizontalCRS": {
    "id": "TESTGCS10",
    "description" : "USES CUSTOM DATUM (TESTDATUM3)",
    "source" : "EPSG:2165",
    "deprecated" : false,
    "epsg" : 0,
    "datumId" : "TEST-GRID2",
    "datum" : {
        "id": "TEST-GRID2",
        "description" : "TEST DATUM 3 - Uses custom ell but custom transfo",
        "deprecated" : false,
        "source" : "Emmo",
        "epsg" : 0,
        "ellipsoidId" : "CustomEllipsoid1",
        "ellipsoid" : {
            "id": "CustomEllipsoid1",
             "description" : "Custom Ellipsoid1 Description",
             "source" : "Custom Ellipsoid1 Source",
             "equatorialRadius" : 6378171.0,
             "polarRadius" : 6356795.719195306
        },
        "transforms": [
        {
            "method": "GridFiles",
            "sourceEllipsoidId" : "CustomEllipsoid1",
            "sourceEllipsoid" : {
                "id": "CustomEllipsoid1",
                "equatorialRadius" : 6378171.0,
                "polarRadius" : 6356795.719195306
            },
            "targetEllipsoidId" : "WGS84",
                "targetEllipsoid" : {
                "id": "WGS84"
            },
            "gridFile" : {
                "files": [
                {
                    "fileName": "./MiddleLandNearMordor/County/Underhill/TooLongAPathThatCannotBeStroedInADGNFIleUnderAnyCircumstances/IA66National(13.09.01).gsb",
                    "format" : "NTv2",
                    "direction" : "Direct"
                }
                ]
            }
        }
        ]
    },
    "unit": "Meter",
    "projection" : {
        "method": "TransverseMercator",
        "centralMeridian" : 143,
        "latitudeOfOrigin" : 0,
        "scaleFactor" : 0.9996,
        "falseEasting" : 500000.0,
        "falseNorthing" : 10000000.0
    },
    "extent": {
        "southWest": {
            "latitude": 3.9,
            "longitude": -7.55
        },
        "northEast": {
            "latitude": 5.13,
            "longitude": -2.75
        }
    }
},
"verticalCRS" : {
    "id" : "GEOID"
}} )X"
};
