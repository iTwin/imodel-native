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
* @bsi                                                   Alain.Robert  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalConversionTest
    {
    Utf8String m_sourceGCS;
    Utf8String m_targetGCS;
    Utf8String m_sourceVerticalDatum;
    Utf8String m_targetVerticalDatum;
    double  m_inputCoordinateX;
    double  m_inputCoordinateY;
    double  m_inputCoordinateZ;
    double  m_outputCoordinateZ;
    double  m_toleranceZ;
    };
 
// Preparation of required environment
class SpecificLegacyVerticalTransformTests : public ::testing::TestWithParam< verticalConversionTest >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        SpecificLegacyVerticalTransformTests() {};
        ~SpecificLegacyVerticalTransformTests() {};
    };

static bvector<verticalConversionTest> s_listOfLegacyVerticalConversionTests = 
    {
    // Global
    {"LL84", "LL84", "ELLIPSOID", "GEOID", 138.727364, 35.360625, 0.0, -41.25, 0.001},          // Mt Fuji, Japan
    // North America
    {"EPSG:4269", "EPSG:4269", "GEOID", "NAVD88", -122.32082778, 47.59364167, 0.0, 0.0, 0.001},           // The legacy behaviour of NAVD88 is to consider it to be the equivalent to GEOID
    {"EPSG:4269", "EPSG:4269", "NGVD29", "NAVD88", -122.32082778, 47.59364167, 0.0, 1.095, 0.001},        // Seattle, USA
    {"EPSG:4269", "EPSG:4269", "NGVD29", "GEOID", -122.32082778, 47.59364167, 0.0, 1.095, 0.001},         // Should be same as above as for PowerPlatform NAVD88 is equivalent to GEOID
    };

/*---------------------------------------------------------------------------------**//**
* Vertical conversion tests using legacy key names and BaseGCS::SetVerticalDatumByKey()
* @bsimethod                                                    Sarah.Keenan  2024-07
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(SpecificLegacyVerticalTransformTests, SpecificLegacyVerticalCoordConversionTest)
{
    verticalConversionTest theConversionTestParam = GetParam(); 

    GeoCoordinates::BaseGCSPtr sourceGCS = GeoCoordinates::BaseGCS::CreateGCS(theConversionTestParam.m_sourceGCS.c_str());
    GeoCoordinates::BaseGCSPtr targetGCS = GeoCoordinates::BaseGCS::CreateGCS(theConversionTestParam.m_targetGCS.c_str());

    ASSERT_TRUE(sourceGCS.IsValid() && sourceGCS->IsValid());
    ASSERT_TRUE(targetGCS.IsValid() && targetGCS->IsValid());

    ASSERT_TRUE(SUCCESS == sourceGCS->SetVerticalDatumByKey(theConversionTestParam.m_sourceVerticalDatum.c_str()));
    ASSERT_TRUE(SUCCESS == targetGCS->SetVerticalDatumByKey(theConversionTestParam.m_targetVerticalDatum.c_str()));

    DPoint3d resultPoint = {0,0,0};
    DPoint3d inputPoint;

    inputPoint.x = theConversionTestParam.m_inputCoordinateX;
    inputPoint.y = theConversionTestParam.m_inputCoordinateY;
    inputPoint.z = theConversionTestParam.m_inputCoordinateZ;

    // forward test
    EXPECT_TRUE(REPROJECT_Success == sourceGCS->CartesianFromCartesian(resultPoint, inputPoint, *targetGCS));

    EXPECT_NEAR(resultPoint.z, theConversionTestParam.m_outputCoordinateZ, theConversionTestParam.m_toleranceZ);

    // reverse test
    EXPECT_TRUE(REPROJECT_Success == targetGCS->CartesianFromCartesian(inputPoint, resultPoint, *sourceGCS));

    EXPECT_NEAR(inputPoint.z, theConversionTestParam.m_inputCoordinateZ, theConversionTestParam.m_toleranceZ);
}


INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         SpecificLegacyVerticalTransformTests,
                         ValuesIn(s_listOfLegacyVerticalConversionTests));

// Preparation of required environment
class SpecificVerticalTransformTests : public ::testing::TestWithParam< verticalConversionTest >
{   
public:
    virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
    virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

    SpecificVerticalTransformTests() {};
    ~SpecificVerticalTransformTests() {};
};

// Tests that use dictioanary name and not legacy key like the tests listed above
static bvector<verticalConversionTest> s_listOfVerticalConversionTests = 
{
    //// EM2008 height / EGM96 height
    // Note that the EGM2008 height values were generated using "interp_2p5min.exe" that is incuded with the download for EGM2008,
    // in comments CS_egm2008.c it states:
    // "... the official tool interp_2p5min.exe uses spline interpolation method, and we use bi-linear here. So the calculated result is slightly"
    // "different from the result generated by interp_2p5min.exe. But it is almost the same as the result generated by PE and Proj. The tolerance is"
    // "less than 1 mm."
    // Bi-linear elevation values for EGM96-15 taken from: https://www.unavco.org/software/geodetic-utilities/geoid-height-calculator/geoid-height-calculator.html
    // EGM2008 values from https://geographiclib.sourceforge.io/cgi-bin/GeoidEval?input=-22.900271+-43.105807&option=Submit which are bi-cubic (we use b-linear)
    {"LL84",   "LL84",    "EGM96 height",    "WGS84",           23.700523,  37.944210,  0.0,    38.3,               0.01},
    {"LL84",   "LL84",    "EGM2008 height",  "WGS84",           23.700523,  37.944210,  0.0,    38.44,              0.01},
    {"LL84",   "LL84",    "EGM96 height",    "EGM2008 height",  23.700523,  37.944210,  0.0,    (38.3-38.44),       0.02},
    {"LL84",   "LL84",    "EGM96 height",    "WGS84",           -43.105807, -22.900271, 0.0,    -5.49,              0.01},
    {"LL84",   "LL84",    "EGM2008 height",  "WGS84",           -43.105807, -22.900271, 0.0,    -5.89,              0.01},
    {"LL84",   "LL84",    "EGM96 height",    "EGM2008 height",  -43.105807, -22.900271, 0.0,    (-5.49-(-5.89)),    0.02},

    //// North America    
    // Not currently in dictionary
//    {"LL84", "LL84", "PRVD02", "WGS84", -66.07701944, 18.43644444, 0.0, 0.0, 0.02},             // San Juan, Puerto Rico
//    {"LL84", "LL84", "VIVD09", "EGM96 height", -64.70516111, 18.33030278, 0.0, 0.0, 0.02},             // Calabash Boom, St John, BVI
//    {"LL84", "LL84", "GUVD04", "EGM96 height", 144.80292222, 13.44996667, 0.0, 0.0, 0.02},             // Mangilao, Guam
//    {"LL84", "LL84", "NMVD03", "NAVD88", 145.75875278, 15.16281389, 0.0, 0.0, 0.02},            // Laulau Bay, Saipan, Northen Mariana Islands
    // Australia
    // Not currently in dictionary
//    {"LL84", "LL84", "ASVD02", "ELLIPSOID", -170.70370556, -14.27232222, 0.0, 0.0, 0.02},     // Pago Pago, American Samoa

    //// Indian Ocean
    {"LL84", "LL84", "Kiunga", "EGM2008 height", 143.88666667, -6.04393611, 0.0, -3.0, 0.002},         // Mt Giluwe, Kiunga, Papua New Guinea

    //// Canada
    // test heights generated using GPS.H from Government Canada https://webapp.csrs-scrs.nrcan-rncan.gc.ca/geod/tools-outils/gpsh.php
    {"LL84", "LL84", "WGS84", "EGM96 height",  -70.93445833, 47.08083056, 775.0, 802.59, 0.02},
    {"LL84", "LL84", "EGM96 height", "CGVD28 height", -70.93445833, 47.08083056, 802.59, 802.826, 0.02},              // Mont Sainte-Anne, Quebec, Canada
    {"LL84", "LL84", "CGVD28 height", "CGVD2013(CGG2013a) height", -70.93445833, 47.08083056, 802.826, 802.483, 0.02},
    {"LL84", "LL84", "CGVD2013(CGG2013a) height", "WGS84", -70.93445833, 47.08083056, 802.483, 775.0, 0.02},
    {"LL84", "LL84", "EGM96 height", "CGVD2013(CGG2013a) height", -70.93445833, 47.08083056, 802.59, 802.483, 0.02},
    // test height generated using vdatum (NOAA Vertical Datum Transformation v4.7.0)
    {"LL84", "LL84", "CGVD2013(CGG2013a) height", "NAVD88 height", -70.93445833, 47.08083056, 802.483, 802.760, 0.02},

    //// UK
    // https://www.ordnancesurvey.co.uk/gps/transformation/
    {"LL84", "LL84", "OSGM15", "WGS84", -0.00147222, 51.47788889, 0.439, 45.8, 0.02},   // Prime Meridian (Greenwich), England 538883.03 Easting, 177330.161 Northing
    {"LL84", "LL84", "OSGM02", "WGS84", -0.00147222, 51.47788889, 0.413, 45.8, 0.02},
    {"LL84", "LL84", "OSGM02", "OSGM15", -0.00147222, 51.47788889, 0.413, 0.439, 0.02},

    {"LL84", "LL84", "OSGM15", "WGS84", -4.290529,  51.595513, 8.398, 61.54, 0.02},     // Llangennith, Wales, 241444.055 Easting, 191004.136 Northing
    {"LL84", "LL84", "OSGM02", "WGS84", -4.290529,  51.595513, 8.376, 61.54, 0.02},
    {"LL84", "LL84", "OSGM02", "OSGM15", -4.290529,  51.595513, 8.376, 8.398, 0.02},

    {"LL84", "LL84", "OSGM15", "WGS84", -4.480016,  57.337451, 0.346, 54.22, 0.02},     // Drunmadrochit, Scotland, 250824.44 Easting, 830159.177 Northing
    {"LL84", "LL84", "OSGM02", "WGS84", -4.480016,  57.337451, 0.318, 54.22, 0.02},
    {"LL84", "LL84", "OSGM02", "OSGM15", -4.480016,  57.337451, 0.318, 0.346, 0.02},

    {"LL84", "LL84", "OSGM15", "WGS84", 1.684774, 52.330884, 0.251, 44.74, 0.02},       // Southwold, England, Easting 651151.265, 276700.732 Northing
    {"LL84", "LL84", "OSGM02", "WGS84", 1.684774, 52.330884, 0.234, 44.74, 0.02},
    {"LL84", "LL84", "OSGM02", "OSGM15", 1.684774, 52.330884, 0.234, 0.251, 0.02},

    {"LL84", "LL84", "WGS84", "NAP2018", 5.387203657, 52.155172897, 43.2551, 0.000, 0.02},

    // 'NAVD88 height' and legacy 'NAVD88' should be the same, 'NAVD88 height' should not be the same as 'NAVD88(Geoid12b) height' which is based on Geoid12b
    {"LL83", "LL83", "NAVD88", "NAVD88 height", -101.24533, 38.45625, 0.0, 0.0, 0.02}, // should always be the same
    {"LL83", "LL83", "WGS84", "EGM2008 height", -101.24533, 38.45625, 0.0, 24.73, 0.02},
    {"LL83", "LL83", "WGS84", "NAVD88(Geoid12b) height", -101.24533, 38.45625, 0.0, 24.375, 0.02},
    {"LL83", "LL83", "EGM2008 height", "NAVD88(Geoid12b) height", -101.24533, 38.45625, 24.73, 24.375, 0.02},

    // Sweden RH2000 height, points from https://www.lantmateriet.se/en/geodata/gps-geodesy-and-swepos/transformations/contentassets/control_points_swen17_bilinear.pdf
    // with SWEREF 99 height assumed to be conincident to ETRS89
    {"LL84", "SWEREF1999-TM", "ETRS89", "RH2000 height", 11.21842314, 59.05709091, 57.5830, 20.6581, 0.02},
    {"LL84", "SWEREF1999-TM", "ETRS89", "RH2000 height", 12.61834981, 60.38649869, 318.0094, 284.2693, 0.02},

    // Latvia quasi-geoid model https://www.lgia.gov.lv/en/latvian-quasi-geoid-model 
    {"LL84", "LL84", "ETRS89", "Latvia 2000 height", 24.3640835, 57.7184762, 24.16, 4.84, 0.02},        // Pt id: 8248 https://geodezija.lgia.gov.lv/VGT/
    {"LL84", "LL84", "ETRS89", "Latvia 2000 height", 27.51917341, 56.52335236, 178.50, 158.66, 0.02},   // Pt id: 1242 https://geodezija.lgia.gov.lv/VGT/
};

/*---------------------------------------------------------------------------------**//**
* Vertical conversion tests using legacy key names and BaseGCS::SetVerticalDatumByKey()
* @bsimethod                                                    Sarah.Keenan  2024-07
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(SpecificVerticalTransformTests, SpecificVerticalCoordConversionTest)
{
    verticalConversionTest theConversionTestParam = GetParam(); 


    GeoCoordinates::BaseGCSPtr sourceGCS = GeoCoordinates::BaseGCS::CreateGCS(theConversionTestParam.m_sourceGCS.c_str());
    GeoCoordinates::BaseGCSPtr targetGCS = GeoCoordinates::BaseGCS::CreateGCS(theConversionTestParam.m_targetGCS.c_str());

    ASSERT_TRUE(sourceGCS.IsValid() && sourceGCS->IsValid());
    ASSERT_TRUE(targetGCS.IsValid() && targetGCS->IsValid());

    ASSERT_TRUE(SUCCESS == sourceGCS->SetVerticalDatumFromName(theConversionTestParam.m_sourceVerticalDatum.c_str()));
    ASSERT_TRUE(SUCCESS == targetGCS->SetVerticalDatumFromName(theConversionTestParam.m_targetVerticalDatum.c_str()));

    DPoint3d resultPoint = {0,0,0};
    DPoint3d inputPoint;

    inputPoint.x = theConversionTestParam.m_inputCoordinateX;
    inputPoint.y = theConversionTestParam.m_inputCoordinateY;
    inputPoint.z = theConversionTestParam.m_inputCoordinateZ;

    // forward test
    EXPECT_EQ(REPROJECT_Success, sourceGCS->CartesianFromCartesian(resultPoint, inputPoint, *targetGCS));

    EXPECT_NEAR(resultPoint.z, theConversionTestParam.m_outputCoordinateZ, theConversionTestParam.m_toleranceZ);

    // reverse test
    EXPECT_TRUE(REPROJECT_Success == targetGCS->CartesianFromCartesian(inputPoint, resultPoint, *sourceGCS));

    EXPECT_NEAR(inputPoint.z, theConversionTestParam.m_inputCoordinateZ, theConversionTestParam.m_toleranceZ);
}

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         SpecificVerticalTransformTests,
                         ValuesIn(s_listOfVerticalConversionTests));
