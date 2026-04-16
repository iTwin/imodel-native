//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#include <Bentley/BeTest.h>

#include <GeoCoord/BaseGeoCoord.h>
#include "GeoCoordTestCommon.h"

using namespace ::testing;

/*---------------------------------------------------------------------------------**//**
* @bsi                                                   Sarah.Keenan   09/2024
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalQueryPointTest
    {
    GeoPoint2d          m_testPt;
    StatusInt           m_testStatus;
    int                 m_minNumberAvailableVerticalDatums;
    bvector<Utf8String>    m_expectedFoundVerticalDatums;
    };
 
// Preparation of required environment
class VerticalDatumUnitTestsQueryPoint : public ::testing::TestWithParam< verticalQueryPointTest >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        VerticalDatumUnitTestsQueryPoint() {};
        ~VerticalDatumUnitTestsQueryPoint() {};
    };

static bvector<verticalQueryPointTest> s_listOfPointTests = 
    {
        // {pt long, lat}, status, minimum number of applicable vertical datums found, some named datums expected to be found
        { {0.0, 0.0},       SUCCESS, 2, {"WGS84","EGM96 height"} },
        { {0.0, -90.0},     SUCCESS, 2, {"WGS84","EGM96 height"} },
        { {0.0, 90.0},      SUCCESS, 2, {"WGS84","EGM96 height"} },
        { {-180.0, 0.0},    SUCCESS, 2, {"WGS84","EGM96 height"} },
        { {180.0, 0.0},     SUCCESS, 2, {"WGS84","EGM96 height"} },
        { {0.0, -90.1},     GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
        { {0.0, 90.1},      GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
        { {-180.1, 0.0},    GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
        { {180.1, 0.0},     GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
        { {178.16, 34.34},  SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
        { {-79.0, 42.0},    SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
        { {178.16, 42.0},   SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
        { {-79.0, 34.34},   SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
    };


/*---------------------------------------------------------------------------------**//**
* Query GeoCoord to find Vertical Datums that are availble for a specific position
* lat/long
* @bsimethod                                                    Sarah.Keenan  2024-07
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(VerticalDatumUnitTestsQueryPoint, VerticalDatumPointUnitTest)
{
    verticalQueryPointTest theTestParam = GetParam(); 

    GeoCoordinates::BaseGCSPtr testGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    bvector<Utf8String> verticalDatums;
    StatusInt status = testGCS->QueryVerticalDatumsAvailableAtPoint(verticalDatums, theTestParam.m_testPt.longitude, theTestParam.m_testPt.latitude);
    ASSERT_TRUE(theTestParam.m_testStatus == status);
    ASSERT_TRUE(theTestParam.m_minNumberAvailableVerticalDatums <= verticalDatums.size());
    int numFound = 0;
    for (const auto& testDatumName : theTestParam.m_expectedFoundVerticalDatums)
    {
        for (const auto& datumName : verticalDatums)
        {
            if (0 == datumName.CompareToI(testDatumName))
            {
                numFound++;
                break;
            }
        }
    }
    ASSERT_TRUE(numFound == theTestParam.m_expectedFoundVerticalDatums.size());
}

/*---------------------------------------------------------------------------------**//**
* @bsi                                                   Sarah.Keenan   09/2024
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalQueryExtentTest
{
    GeoPoint2d          m_testMinExtent;
    GeoPoint2d          m_testMaxExtent;
    StatusInt           m_testStatus;
    int                 m_minNumberAvailableVerticalDatums;
    bvector<Utf8String>    m_expectedFoundVerticalDatums;
};

// Preparation of required environment
class VerticalDatumUnitTestsQueryExtent : public ::testing::TestWithParam< verticalQueryExtentTest >
{   
public:
    virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
    virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

    VerticalDatumUnitTestsQueryExtent() {};
    ~VerticalDatumUnitTestsQueryExtent() {};
};

static bvector<verticalQueryExtentTest> s_listOfExtentTests = 
{
    // {range long min, lat min}, {range long max, lat max}, status, minimum number of applicable vertical datums found, a named datum expected to be found
    { {0.0, 0.0},       {0.0, 0.0},         SUCCESS, 2, {"WGS84","EGM96 height"} },
    { {0.0, -90.0},     {0.0, 0.0},         SUCCESS, 2, {"WGS84","EGM96 height"} },
    { {0.0, 0.0},       {0.0, 90.0},        SUCCESS, 2, {"WGS84","EGM96 height"} },
    { {-180.0, 0.0},    {0.0, 0.0},         SUCCESS, 2, {"WGS84","EGM96 height"} },
    { {0.0, 0.0},       {180.0, 0.0},       SUCCESS, 2, {"WGS84","EGM96 height"} },
    { {0.0, -90.1},     {0.0, 0.0},         GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
    { {0.0, 0.0},       {0.0, 90.1},        GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
    { {-180.1, 0.0},    {0.0, 0.0},         GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
    { {0.0, 0.0},       {180.1, 0.0},       GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
    { {178.10, 34.30},  {178.20, 34.40},    SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
    { {-79.0, 42.0},    {-78.0, 43.0},      SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
    { {178.16, 42.0},   {179.16, 41.0},     SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
    { {-79.1, 34.30},   {-79.0, 34.40},     SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
    { {178.16, 42.0},   {-179.16, 41.0},    SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },

};

/*---------------------------------------------------------------------------------**//**
* Query GeoCoord to find Vertical Datums that are availble for a specific range
* lat/long
* @bsimethod                                                    Sarah.Keenan  2024-07
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(VerticalDatumUnitTestsQueryExtent, VerticalDatumUnitTestsExtent)
{
    verticalQueryExtentTest theTestParam = GetParam(); 

    GeoCoordinates::BaseGCSPtr testGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    bvector<Utf8String> verticalDatums;
    StatusInt status = testGCS->QueryVerticalDatumsAvailableForRange(verticalDatums, theTestParam.m_testMinExtent.longitude, theTestParam.m_testMinExtent.latitude,
                                                                    theTestParam.m_testMaxExtent.longitude, theTestParam.m_testMaxExtent.latitude, false);
    ASSERT_TRUE(theTestParam.m_testStatus == status);
    ASSERT_TRUE(theTestParam.m_minNumberAvailableVerticalDatums <= verticalDatums.size());
    int numFound = 0;
    for (const auto& testDatumName : theTestParam.m_expectedFoundVerticalDatums)
    {
        for (const auto& datumName : verticalDatums)
        {
            if (0 == datumName.CompareToI(testDatumName))
            {
                numFound++;
                break;
            }
        }
    }
    ASSERT_TRUE(numFound == theTestParam.m_expectedFoundVerticalDatums.size());

}

/*---------------------------------------------------------------------------------**//**
* @bsi                                                   Sarah.Keenan   09/2024
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalDatumEqualTest
{
    Utf8String     m_verticalDatumName1;
    Utf8String     m_verticalDatumName2;
    bool        m_equal;
};

// Preparation of required environment
class VerticalDatumUnitTestEquivalence : public ::testing::TestWithParam< verticalDatumEqualTest >
{   
public:
    virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
    virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

    VerticalDatumUnitTestEquivalence() {};
    ~VerticalDatumUnitTestEquivalence() {};
};

static bvector<verticalDatumEqualTest> s_listOfVerticalDatumEqualTests = 
{
    // {vertical datum name in dictionary}, {name of vertical datum to compare with}, vertical datums are equal
    // Note: the reason for all the equal items is to test the operator==()/IsEqualTo() methods
    // for VerticalDatum, VerticalDatumInfo, VerticalTransform and VerticalTransformPathInfo
    { "WGS84",                 "WGS84",           true },
    { "EGM96 height",          "EGM96 height",    true },
    { "EGM2008 height",        "EGM2008 height",  true },
    { "NAVD88 height",         "NAVD88 height",   true },
    { "NGVD29 height",         "NGVD29 height",   true },
    { "Kiunga",                "Kiunga",          true },
    { "NAVD88 height",         "NGVD29 height",   false },
};

/*---------------------------------------------------------------------------------**//**
* Equivalence test
* @bsimethod                                                    Sarah.Keenan  2024-07
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(VerticalDatumUnitTestEquivalence, VerticalDatumUnitTestEqual)
{
    verticalDatumEqualTest theTestParam = GetParam(); 

    StatusInt status1, status2;
    GeoCoordinates::VerticalDatumPtr verticalDatum1 = GeoCoordinates::BaseGCS::CreateVerticalDatumFromName(theTestParam.m_verticalDatumName1.c_str(), status1);
    GeoCoordinates::VerticalDatumPtr verticalDatum2 = GeoCoordinates::BaseGCS::CreateVerticalDatumFromName(theTestParam.m_verticalDatumName2.c_str(), status2);

    ASSERT_TRUE(verticalDatum1.IsValid());
    ASSERT_TRUE(verticalDatum2.IsValid());
    ASSERT_TRUE(theTestParam.m_equal == (*(verticalDatum1.get()) == *(verticalDatum2.get())));
}

/*---------------------------------------------------------------------------------**//**
* @bsi                                                   Sarah.Keenan   09/2024
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalDatumInitFromNameTest
{
    Utf8String     m_verticalDatumName;
    Utf8String     m_crsName;
    bool        m_canInitializeUsingName;
};

// Preparation of required environment
class VerticalDatumUnitTestInitFromName : public ::testing::TestWithParam< verticalDatumInitFromNameTest >
{   
public:
    virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
    virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

    VerticalDatumUnitTestInitFromName() {};
    ~VerticalDatumUnitTestInitFromName() {};
};

static bvector<verticalDatumInitFromNameTest> s_listOfVerticalDatumInitFromNameTests = 
{
    // crs names not in dictionary, legacy names, can create new dictionary vertical datum if equivalent available
    { "LOCAL_ELLIPSOID", "BritishNatGrid", false },
    { "ELLIPSOID", "LL84", true },    // Creates 'WGS84'
    { "GEOID", "LL84", false },
    { "NAVD88", "CT83", true },       // Creates 'NAVD88 height'
    { "NGVD29", "CT83", true },       // Creates 'NGVD29 height'
    // crs names in dictionary, should be able to create new dictionary vertical datum
    { "WGS84", "LL84", true },
    { "NAVD88 height", "LL84", true },
    { "NAVD88(Geoid12b) height", "LL84", true },
    { "NGVD29 height", "LL84", true },
    { "EGM96 height", "LL84", true },
    { "EGM2008 height", "LL84", true },
    { "Kiunga", "LL84", true },
    { "CGVD28 height", "LL84", true },
    { "CGVD2013(CGG2013a) height", "LL84", true },
    { "OSGM15", "LL84", true },
    { "NAP2018", "LL84", true },
    { "OSGM02", "LL84", true }
};

/*---------------------------------------------------------------------------------**//**
* Set vertical datum using Name test vs set using legacy key, the legacy keys should never
* be available in the vertical datum dictionary so setting by name should fail for those
* @bsimethod                                                    Sarah.Keenan  2024-11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(VerticalDatumUnitTestInitFromName, VerticalDatumUnitTestInitFromName)
{
    verticalDatumInitFromNameTest theTestParam = GetParam(); 

    // init using vertical datum dictionary
    GeoCoordinates::BaseGCSPtr gcs1 = GeoCoordinates::BaseGCS::CreateGCS(theTestParam.m_crsName.c_str());
    gcs1->SetVerticalDatumFromName(theTestParam.m_verticalDatumName.c_str());
    ASSERT_TRUE(theTestParam.m_canInitializeUsingName == gcs1->HasValidVerticalDatum());
}

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumUnitTestsQueryPoint,
                         ValuesIn(s_listOfPointTests));

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumUnitTestsQueryExtent,
                         ValuesIn(s_listOfExtentTests));

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumUnitTestEquivalence,
                         ValuesIn(s_listOfVerticalDatumEqualTests));

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumUnitTestInitFromName,
                         ValuesIn(s_listOfVerticalDatumInitFromNameTests));
