//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------
#include <Bentley/BeTest.h>

#include <GeoCoord/BaseGeoCoord.h>
#include "GeoCoordTestCommon.h"

using namespace ::testing;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct conversionTestCustom
    {
    Utf8String m_sourceGCSJson;
    Utf8String m_targetGCSJson;
    double  m_inputCoordinateX;
    double  m_inputCoordinateY;
    double  m_inputCoordinateZ;
    double  m_outputCoordinateX;
    double  m_outputCoordinateY;
    double  m_outputCoordinateZ;
    double  m_toleranceDirect;
    double  m_toleranceBack;
    };
 
// Preparation of required environment
class SpecificCustomTransformTests : public ::testing::TestWithParam< conversionTestCustom >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        SpecificCustomTransformTests() {};
        ~SpecificCustomTransformTests() {};
    };

static bvector<conversionTestCustom> s_listOfConversionTestsCustom = 
    {
    {R"X({"horizontalCRS": { "id": "BritishNatGrid"}, "verticalCRS": {"id": "ELLIPSOID"}})X", R"X({ "horizontalCRS": { "id": "OSGB.LL"}, "verticalCRS": {"id": "ELLIPSOID"}})X", 170370.71800000000000, 11572.40500000000000, 0.0, -5.2020119082059511, 49.959453295440234, 0.0, 0.00000002, 0.002},
  
    {R"X({ "horizontalCRS": { "id": "BritishNatGrid"}, "verticalCRS": {"id": "ELLIPSOID"} })X", R"X({ "horizontalCRS": { "id": "LL84"}, "verticalCRS": {"id": "ELLIPSOID"} })X", 170370.71800000000000, 11572.40500000000000, 0.0, -5.2020119082059511, 49.959453295440234, 0.0, 0.002, 0.002},
    { R"X( { "horizontalCRS": { "id": "UTM83-10"}, "verticalCRS": {"id": "NGVD29"} })X", R"X({ "horizontalCRS": { "id": "LL84"}, "verticalCRS": {"id": "ELLIPSOID"} })X", 632748.112, 4263868.307, 0.0, -121.47738265889652, 38.513305313793019, -30.12668428839329, 0.002, 0.002},

    { R"X( { "horizontalCRS": { "id": "UTM83-10"}, "verticalCRS": {"id": "NGVD29"} })X", R"X({ "horizontalCRS": { "id": "LL84"}, "verticalCRS": {"id": "GEOID"} })X", 632748.112, 4263868.307, 0.0, -121.47738265889652, 38.513305313793019, 0.7621583779125531, 0.002, 0.002},
    { R"X( { "horizontalCRS": { "id": "UTM83-10"}, "verticalCRS": {"id": "NGVD29"} })X", R"X({ "horizontalCRS": { "id": "CA83-II"}, "verticalCRS": {"id": "NAVD88"} })X", 569024.940, 4386341.752, 0.0, 1983192.529823256, 717304.0311293667, 0.745910484422781, 0.002, 0.002},
    { R"X( { "horizontalCRS": { "id": "UTM83-10"}, "verticalCRS": {"id": "NGVD29"} })X", R"X({ "horizontalCRS": { "id": "CA83-II"}, "verticalCRS": {"id": "GEOID"} })X", 569024.940, 4386341.752, 0.0, 1983192.529823256, 717304.0311293667, 0.745910484422781, 0.002, 0.002},
    { R"X( { "horizontalCRS": { "id": "UTM83-10"}, "verticalCRS": {"id": "NGVD29"} })X", R"X({ "horizontalCRS": { "id": "CA83-II"}, "verticalCRS": {"id": "NGVD29"} })X", 569024.940, 4386341.752, 0.0, 1983192.529823256, 717304.0311293667, 0.0, 0.002, 0.002},
    { R"X( { "horizontalCRS": { "id": "UTM83-10"}, "verticalCRS": {"id": "NGVD29"} })X", R"X({ "horizontalCRS": { "epsg": 26942}, "verticalCRS": {"id": "NAVD88"} })X", 569024.940, 4386341.752, 0.0, 1983192.529823256, 717304.0311293667, 0.745910484422781, 0.002, 0.002},
    //{ R"X( { "horizontalCRS": { "id": "UTM83-10"}, "verticalCRS": {"id": "NGVD29"} })X", R"X({ "horizontalCRS": { "epsg": 6418}, "verticalCRS": {"id": "NAVD88"} })X", 569024.940, 4386341.752, 0.0, 6506524.158595133, 2353354.975796927, 2.4472079809770739, 0.002, 0.002},
   // { R"X( { "horizontalCRS": { "id": "UTM83-10"}, "verticalCRS": {"id": "NGVD29"} })X", R"X({ "horizontalCRS": { "id": "CA83/2011-IIF" }, "verticalCRS": {"id": "NAVD88"} })X", 569024.940, 4386341.752, 0.0, 6506524.158595133, 2353354.975796927, 2.4472079809770739, 0.002, 0.002},

    { R"X({ "horizontalCRS": { "id": "BritishNatGrid"}, "verticalCRS": {"id": "ELLIPSOID"} })X", R"X({ "horizontalCRS": { "id": "HS2_Snake_2015" }, "verticalCRS": {"id": "GEOID"} })X", 473327.251, 257049.636, 0.0, 237732.58101946692, 364048.01547843055, -47.874172425966336, 0.002, 0.002},
    { R"X({ "horizontalCRS": { "id": "OSGB-7P.BritishNatGrid"}, "verticalCRS": {"id": "ELLIPSOID"} })X",
      R"X({
        "horizontalCRS": {
          "id": "HS2-MOCK",
          "description" : "USES CUSTOM DATUM",
          "source" : "Test",
          "deprecated" : false,
          "datumId" : "TEST-GRID1",
          "datum" : {
            "id": "TEST-GRID1",
            "source" : "Emmo",
            "ellipsoidId" : "WGS84",
            "transforms": [
              {
                "method": "GridFiles",
                "sourceEllipsoid" : {
                  "id": "WGS84"},
                "targetEllipsoid" : {
                  "id": "GRS1980"},
                "gridFile": {
                  "files": [
                    { "fileName": "./UK/HS2/HS2TN15_NTv2.gsb", "format": "NTv2", "direction": "Direct" }
                  ]
                }
              }]},
          "unit": "Meter",
          "projection" : {
            "method": "TransverseMercator",
            "centralMeridian": -1.5,
            "latitudeOfOrigin": 52.30,
            "scaleFactor": 1.0,
            "falseEasting": 198873.0046,
            "falseNorthing": 375064.3871}
        },
        "verticalCRS" : {
          "id" : "GEOID"}})X"
      , 473327.251, 257049.636, 0.0, 237733.14012154864, 364046.22617196420, -47.874140587025259, 0.002, 0.002},
#if (0)
    // This particular test fails because when custom datum are involved they cannot inter-mix with non-reversible transformations
    // Such as Multiple Regression implemented in the OSGB datum used by BritishNatGrid
    // A minor known bug to fix eventually.
    { R"X({ "horizontalCRS": { "id": "BritishNatGrid"}, "verticalCRS": {"id": "ELLIPSOID"} })X",
      R"X({
        "horizontalCRS": {
          "id": "HS2-MOCK",
          "description" : "USES CUSTOM DATUM",
          "source" : "Test",
          "deprecated" : false,
          "datumId" : "TEST-GRID1",
          "datum" : {
            "id": "TEST-GRID1",
            "source" : "Emmo",
            "ellipsoidId" : "WGS84",
            "transforms": [
              {
                "method": "GridFiles",
                "sourceEllipsoid" : {
                  "id": "WGS84"},
                "targetEllipsoid" : {
                  "id": "GRS1980"},
                "gridFile": {
                  "files": [
                    { "fileName": "./UK/HS2/HS2TN15_NTv2.gsb", "format": "NTv2", "direction": "Direct" }
                  ]
                }
              }]},
          "unit": "Meter",
          "projection" : {
            "method": "TransverseMercator",
            "centralMeridian": -1.5,
            "latitudeOfOrigin": 52.30,
            "scaleFactor": 1.0,
            "falseEasting": 198873.0046,
            "falseNorthing": 375064.3871}
        },
        "verticalCRS" : {
          "id" : "GEOID"}})X"
      , 473327.251, 257049.636, 0.0, 237732.58101952373, 364048.01548327296, -47.874172425966336, 0.002, 0.002},

#endif

    { R"X({ "horizontalCRS": { "id": "BritishNatGrid"}, "verticalCRS" : {"id": "ELLIPSOID"} })X", R"X({ "horizontalCRS": { "id": "OSGB-GPS-2015" }, "verticalCRS": {"id": "GEOID"} })X", 473327.251, 257049.636, 0.0, 473325.6830048648, 257049.77062273448, -47.87643904264457, 0.002, 0.002},

    { R"X({ "horizontalCRS": { "id": "UTM83-10"}, "verticalCRS": {"id": "NGVD29"} })X",
      R"X({
        "horizontalCRS": {
          "id": "California2",
          "description" : "USES CUSTOM DATUM",
          "source" : "Test",
          "deprecated" : false,
          "datumId" : "TEST-GRID",
          "datum" : {
            "id": "TEST-GRID",
            "description" : "TEST DATUM - Uses custom ell and custom transfo",
            "deprecated" : false,
            "source" : "Emmo",
            "ellipsoidId" : "CustomEllipsoid1",
            "ellipsoid" : {
              "id": "CustomEllipsoid1",
              "description" : "Custom Ellipsoid1 Description",
              "source" : "Custom Ellipsoid1 Source",
              "equatorialRadius" : 6378171.1,
              "polarRadius" : 6356795.719195306},
            "transforms": [
              {
                "method": "Geocentric",
                "sourceEllipsoid" : {
                  "id": "CustomEllipsoid2",
                  "equatorialRadius" : 6378171.1,
                  "polarRadius" : 6356795.719195306},
                "targetEllipsoid" : {
                  "id": "CustomEllipsoid3",
                  "equatorialRadius" : 6378174.1,
                  "polarRadius" : 6356796.1},
                "geocentric" : {
                  "delta": {
                    "x" : -15,
                    "y" : 18,
                    "z" : 46}}},
              {
                "method": "PositionalVector",
                "positionalVector" : {
                  "scalePPM": 2.4985,
                  "delta" : {
                    "x" : -120.271,
                    "y" : -64.543,
                    "z" : 161.632},
                  "rotation" : {
                    "x" : 0.2175,
                    "y" : -0.0672,
                    "z" : -0.1291}},
                "sourceEllipsoid" : {
                  "id": "CustomEllipsoid3",
                  "equatorialRadius" : 6378174.1,
                  "polarRadius" : 6356796.1},
                "targetEllipsoid" : {
                  "id": "WGS84",
                  "equatorialRadius" : 6378137.0,
                  "polarRadius" : 6356752.3142}}]},
          "unit": "Meter",
          "projection" : {
            "method": "LambertConformalConicTwoParallels",
            "longitudeOfOrigin"  : -122,
            "latitudeOfOrigin" : 37.66666666667,
            "standardParallel1" : 39.833333333333336,
            "standardParallel2" : 38.333333333333334,
            "falseEasting" : 2000000.0,
            "falseNorthing" : 500000.0},
          "extent": {
            "southWest": {
              "latitude": 35,
              "longitude": -125},
            "northEast": {
              "latitude": 39.1,
              "longitude": -120.45}}},
        "verticalCRS" : {
          "id" : "GEOID"}})X", 632748.112, 4263868.307, 0.0, 2045672.959210648, 594018.471211601, 0.7621583779125531, 0.002, 0.004},

    // The following two are functionaly identical but the second one is fully custom and uses additional transform path as definition.
    { R"X( { "horizontalCRS": { "id": "UTM83/2011-10"}, "verticalCRS": {"id": "NGVD29"} })X", R"X({ "horizontalCRS": { "id": "CA-II"}, "verticalCRS": {"id": "GEOID"} })X",  632748.112, 4263868.307, 0.0, 2149846.770, 308797.585, 2.500, 0.002, 0.004},

    { R"X({ "horizontalCRS": { "id": "UTM83/2011-10"}, "verticalCRS": {"id": "NGVD29"} })X",
      R"X({
        "horizontalCRS": {
          "id": "California2A",
          "description" : "USES Dummy NAD27",
          "source" : "Test",
          "deprecated" : false,
          "datumId" : "NAD27A",
          "datum" : {
             "additionalTransformPaths" : [
                {
                   "sourceDatumId" : "NAD27A",
                   "targetDatumId" : "NAD83/2011",
                   "transforms" : [
                      {
                         "gridFile" : {
                            "files" : [
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Nadcon/conus.l?s",
                                  "format" : "NADCON"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Nadcon/alaska.l?s",
                                  "format" : "NADCON"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Nadcon/prvi.l?s",
                                  "format" : "NADCON"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Nadcon/hawaii.l?s",
                                  "format" : "NADCON"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Nadcon/stgeorge.l?s",
                                  "format" : "NADCON"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Nadcon/stlrnc.l?s",
                                  "format" : "NADCON"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Nadcon/stpaul.l?s",
                                  "format" : "NADCON"
                               }
                            ]
                         },
                         "method" : "GridFiles",
                         "sourceEllipsoid" : {
                            "equatorialRadius" : 6378206.4000000004,
                            "id" : "CLRK66",
                            "polarRadius" : 6356583.7999999998
                         },
                         "targetDatumId" : "NAD83A",
                         "targetEllipsoid" : {
                            "equatorialRadius" : 6378137.0,
                            "id" : "GRS1980",
                            "polarRadius" : 6356752.3141403478
                         }
                      },
                      {
                         "gridFile" : {
                            "files" : [
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Harn/48hpgn.l?s",
                                  "format" : "NADCON"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Harn/hihpgn.l?s",
                                  "format" : "NADCON"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Harn/pvhpgn.l?s",
                                  "format" : "NADCON"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Harn/eshpgn.l?s",
                                  "format" : "NADCON"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/Harn/wshpgn.l?s",
                                  "format" : "NADCON"
                               }
                            ]
                         },
                         "method" : "GridFiles",
                         "sourceEllipsoid" : {
                            "equatorialRadius" : 6378137.0,
                            "id" : "GRS1980",
                            "polarRadius" : 6356752.3141403478
                         },
                         "targetDatumId" : "NAD83/HARN-B",
                         "targetEllipsoid" : {
                            "equatorialRadius" : 6378137.0,
                            "id" : "GRS1980",
                            "polarRadius" : 6356752.3141403478
                         }
                      },
                      {
                         "gridFile" : {
                            "files" : [
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/NSRS2007/dsl?.b",
                                  "format" : "GEOCN"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/NSRS2007/dsl?a.b",
                                  "format" : "GEOCN"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/NSRS2007/dsl?p.b",
                                  "format" : "GEOCN"
                               }
                            ]
                         },
                         "method" : "GridFiles",
                         "sourceEllipsoid" : {
                            "equatorialRadius" : 6378137.0,
                            "id" : "GRS1980",
                            "polarRadius" : 6356752.3141403478
                         },
                         "targetDatumId" : "NSRS07A",
                         "targetEllipsoid" : {
                            "equatorialRadius" : 6378137.0,
                            "id" : "GRS1980",
                            "polarRadius" : 6356752.3141403478
                         }
                      },
                      {
                         "gridFile" : {
                            "files" : [
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/NSRS2011/dsl?11.b",
                                  "format" : "GEOCN"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/NSRS2011/dsl?a11.b",
                                  "format" : "GEOCN"
                               },
                               {
                                  "direction" : "Direct",
                                  "fileName" : "./Usa/NSRS2011/dsl?p11.b",
                                  "format" : "GEOCN"
                               }
                            ]
                         },
                         "method" : "GridFiles",
                         "sourceEllipsoid" : {
                            "equatorialRadius" : 6378137.0,
                            "id" : "GRS1980",
                            "polarRadius" : 6356752.3141403478
                         },
                         "targetDatumId" : "NAD83/2011",
                         "targetEllipsoid" : {
                            "equatorialRadius" : 6378137.0,
                            "id" : "GRS1980",
                            "polarRadius" : 6356752.3141403478
                         }
                      }
                   ]
                }
             ],
             "description" : "Dummy NAD27 Look-alike",
             "ellipsoidId" : "CLRK66",
             "id" : "NAD27A",
             "source" : "Dummy",
             "transforms" : [
                {
                   "gridFile" : {
                      "files" : [
                         {
                            "direction" : "Direct",
                            "fileName" : "./Usa/Nadcon/conus.l?s",
                            "format" : "NADCON"
                         },
                         {
                            "direction" : "Direct",
                            "fileName" : "./Usa/Nadcon/alaska.l?s",
                            "format" : "NADCON"
                         },
                         {
                            "direction" : "Direct",
                            "fileName" : "./Usa/Nadcon/prvi.l?s",
                            "format" : "NADCON"
                         },
                         {
                            "direction" : "Direct",
                            "fileName" : "./Usa/Nadcon/hawaii.l?s",
                            "format" : "NADCON"
                         },
                         {
                            "direction" : "Direct",
                            "fileName" : "./Usa/Nadcon/stgeorge.l?s",
                            "format" : "NADCON"
                         },
                         {
                            "direction" : "Direct",
                            "fileName" : "./Usa/Nadcon/stlrnc.l?s",
                            "format" : "NADCON"
                         },
                         {
                            "direction" : "Direct",
                            "fileName" : "./Usa/Nadcon/stpaul.l?s",
                            "format" : "NADCON"
                         }
                      ]
                   },
                   "method" : "GridFiles",
                   "sourceEllipsoid" : {
                      "equatorialRadius" : 6378206.4000000004,
                      "id" : "CLRK66",
                      "polarRadius" : 6356583.7999999998
                   },
                   "targetDatumId" : "NAD83A",
                   "targetEllipsoid" : {
                      "equatorialRadius" : 6378137.0,
                      "id" : "GRS1980",
                      "polarRadius" : 6356752.3141403478
                   }
                }
             ]
          },
          "unit": "USSurveyFoot",
          "projection" : {
            "method": "LambertConformalConicTwoParallels",
            "longitudeOfOrigin"  : -122,
            "latitudeOfOrigin" : 37.66666666667,
            "standardParallel1" : 39.833333333333336,
            "standardParallel2" : 38.333333333333334,
            "falseEasting" : 2000000.0,
            "falseNorthing" : 0.0},
          "extent": {
            "southWest": {
              "latitude": 37.5,
              "longitude": -125},
            "northEast": {
              "latitude": 41.0,
              "longitude": -119.5}}},
        "verticalCRS" : {
          "id" : "GEOID"}})X", 632748.112, 4263868.307, 0.0, 2149846.770, 308797.585, 2.500, 0.002, 0.004},

    };


/*---------------------------------------------------------------------------------**//**
* Parametrized conversion tests.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(SpecificCustomTransformTests, SpecificCoordConversionTest)
{
    conversionTestCustom theConversionTestParam = GetParam(); 

    GeoCoordinates::BaseGCSPtr sourceGCS = GeoCoordinates::BaseGCS::CreateGCS();
    GeoCoordinates::BaseGCSPtr targetGCS = GeoCoordinates::BaseGCS::CreateGCS();

    EXPECT_TRUE(sourceGCS.IsValid());
    EXPECT_TRUE(targetGCS.IsValid());

    Utf8String errorMessage;

    ASSERT_TRUE(SUCCESS == sourceGCS->FromJson(BeJsDocument(theConversionTestParam.m_sourceGCSJson), errorMessage));
    ASSERT_TRUE(SUCCESS == targetGCS->FromJson(BeJsDocument(theConversionTestParam.m_targetGCSJson), errorMessage));

    ASSERT_TRUE(sourceGCS->IsValid());
    ASSERT_TRUE(targetGCS->IsValid());

    DPoint3d resultPoint = { 0,0,0 };
    DPoint3d inputPoint;

    inputPoint.x = theConversionTestParam.m_inputCoordinateX;
    inputPoint.y = theConversionTestParam.m_inputCoordinateY;
    inputPoint.z = theConversionTestParam.m_inputCoordinateZ;

    EXPECT_TRUE(REPROJECT_Success == sourceGCS->CartesianFromCartesian(resultPoint, inputPoint, *targetGCS));

    EXPECT_NEAR(resultPoint.x, theConversionTestParam.m_outputCoordinateX, theConversionTestParam.m_toleranceDirect);
    EXPECT_NEAR(resultPoint.y, theConversionTestParam.m_outputCoordinateY, theConversionTestParam.m_toleranceDirect);
    EXPECT_NEAR(resultPoint.z, theConversionTestParam.m_outputCoordinateZ, theConversionTestParam.m_toleranceDirect);

    DPoint3d returnPoint = { 0,0,0 };
    EXPECT_TRUE(REPROJECT_Success == targetGCS->CartesianFromCartesian(returnPoint, resultPoint, *sourceGCS));

    EXPECT_NEAR(returnPoint.x, theConversionTestParam.m_inputCoordinateX, theConversionTestParam.m_toleranceBack);
    EXPECT_NEAR(returnPoint.y, theConversionTestParam.m_inputCoordinateY, theConversionTestParam.m_toleranceBack);
    EXPECT_NEAR(returnPoint.z, theConversionTestParam.m_inputCoordinateZ, theConversionTestParam.m_toleranceBack);
}

    
INSTANTIATE_TEST_SUITE_P(SpecificCustomTransformTests_Combined,
                        SpecificCustomTransformTests,
                        ValuesIn(s_listOfConversionTestsCustom));


   


