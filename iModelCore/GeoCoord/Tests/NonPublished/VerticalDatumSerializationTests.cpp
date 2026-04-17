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
* @bsi                                                   Sarah.Keenan   10/2024
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalDatumSerializationTest
    {
    Utf8String  m_name;
    Utf8String  m_jsonStr;
    bool        m_equalToDictionaryItem;
    };
 
// Preparation of required environment
class VerticalDatumSerializationTests : public ::testing::TestWithParam< verticalDatumSerializationTest >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        VerticalDatumSerializationTests() {};
        ~VerticalDatumSerializationTests() {};
    };

static bvector<verticalDatumSerializationTest> s_listOfTests = 
    {
        // [dictionary item name] [equivalent Json definition] [should be equal to dictionary item]
        // --> Equal items <---
        {
            "EGM96 height",
            R"X( { 
                "verticalCRS": {
                    "crsName": "EGM96 height",
                    "datumName": "EGM96 geoid",
                    "epsg": 5773,
                    "type": "GEOID",
                    "description": "Earth Geoid Model 1996 with 15 seconds density",
                    "areaOfUse": "World",
                    "remarks": "Zero-height surface resulting from the application of the EGM96 geoid model to the WGS 84 ellipsoid. Replaces EGM84 height (CRS code 5798). Replaced by EGM2008 height (CRS code 3855).",
                    "units" : "meter",
                    "transforms": [
                        {
                            "target" : "WGS84",
                            "geoidSeparationGrid": {
                                "direction": "Direct",
                                "format": "GRD",
                                "files": [
                                    "./World/WW15MGH.GRD"
                                ]
                            }
                        }
                    ],
                    "extent": {
                        "southWest": {
                            "latitude": -90,
                            "longitude": -180},
                        "northEast": {
                            "latitude": 90.0,
                            "longitude": 180.0
                            }
                        }
                }
            } )X",
            true,
        },
        {
            "NGVD29 height",
            R"X( { 
                "verticalCRS": {
                    "crsName": "NGVD29 height",
                    "datumName": "National Geodetic Vertical Datum 1929",
                    "epsg": 7968,
                    "type": "GEOID",
                    "description": "National Geodetic Vertical Datum 1929 - Meter",
                    "areaOfUse": "Continental United States(USA)",
                    "remarks": "Replaced by NAVD88 height.",
                    "units" : "meter",
                    "transforms": [
                        {
                            "target": "NAVD88 height",
                            "vertOffsetGrid": {
                                "direction": "Direct",
                                "format": "VERTCON",
                                "files": [
                                    "./Usa/Vertcon/VERTCONC.94",
                                    "./Usa/Vertcon/VERTCONE.94",
                                    "./Usa/Vertcon/VERTCONW.94"
                                ]
                            }
                        }
                    ],
                    "transformPaths": [
                        {
                            "target" : "WGS84",
                            "path" : [ "NGVD29 height", "NAVD88 height", "EGM96 height", "WGS84" ]
                        },
                        {
                            "target" : "EGM96 height",
                            "path" : [ "NGVD29 height", "NAVD88 height", "EGM96 height" ]
                        }
                    ],
                    "extent": {
                        "southWest": {
                            "latitude": 14.51,
                            "longitude": 172.42
                        },
                        "northEast": {
                            "latitude": 71.4,
                            "longitude": -66.91
                        }
                    }
                }
            } )X",
            true,
        },
        {
            "Kiunga",
            R"X( { 
                "verticalCRS": {
                    "crsName": "Kiunga",
                    "epsg": 7652,
                    "type": "GEOID",
                    "description": "Kiunga Height",
                    "areaOfUse": "Papua New Guinea - onshore south of 5 degrees S and west of 144 degrees E",
                    "remarks": "Kiunga height = WGS 84 ellipsoid height - value of geoid undulation derived by bilinear interpolation of EGM2008 geoid model - 3.0m = EGM2008 height - 3.0m. See CRS code 3855 and transformation code 3858.",
                    "units" : "meter",
                    "transforms": [
                        {
                            "target": "EGM2008 height",
                            "verticalOffset": {
                                "offset": -3,
                                "units" : "meter"
                                }
                        }      
                    ],
                    "extent": {
                        "southWest": {
                            "latitude": -9.35,
                            "longitude": 140.85
                        },
                        "northEast": {
                            "latitude": -5,
                            "longitude": 144.01
                        }
                    }
                }
            } )X",
            true,
        },
        // --> Not equal items (similar but with small changes <---
        // different grid file
        {
            "EGM96 height",
            R"X( { 
                "verticalCRS": {
                "crsName": "EGM96 height",
                "datumName": "EGM96 geoid",
                "epsg": 5773,
                "type": "GEOID",
                "description": "Earth Geoid Model 1996 with 15 seconds density",
                "areaOfUse": "World",
                "remarks": "Zero-height surface resulting from the application of the EGM96 geoid model to the WGS 84 ellipsoid. Replaces EGM84 height (CRS code 5798). Replaced by EGM2008 height (CRS code 3855).",
                "units" : "meter",
                "transforms": [
                    {
                        "target" : "WGS84",
                        "geoidSeparationGrid": {
                            "direction": "Direct",
                            "format": "GRD",
                            "files": [
                                "./World/WW15MGH.GRID"
                            ]
                        }
                    }
                ],
                "extent": {
                    "southWest": {
                        "latitude": -90,
                        "longitude": -180},
                    "northEast": {
                        "latitude": 90.0,
                        "longitude": 180.0
                        }
                    }
                }
            } )X",
            false,
        },
        // different direction
        {
            "NGVD29 height",
            R"X( { 
                "verticalCRS": {
                    "crsName": "NGVD29 height",
                    "datumName": "National Geodetic Vertical Datum 1929",
                    "epsg": 7968,
                    "type": "GEOID",
                    "description": "National Geodetic Vertical Datum 1929 - Meter",
                    "areaOfUse": "Continental United States(USA)",
                    "remarks": "Replaced by NAVD88 height.",
                    "units" : "meter",
                    "transforms": [
                        {
                            "target": "NAVD88 height",
                            "vertOffsetGrid": {
                                "direction": "Inverse",
                                "format": "VERTCON",
                                "files": [
                                    "./Usa/Vertcon/VERTCONC.94",
                                    "./Usa/Vertcon/VERTCONE.94",
                                    "./Usa/Vertcon/VERTCONW.94"
                                ]
                            }
                        }
                   ],
                    "extent": {
                        "southWest": {
                            "latitude": 14.51,
                            "longitude": 172.42
                        },
                        "northEast": {
                            "latitude": 71.4,
                            "longitude": -66.91
                        }
                    }
                }
            } )X",
            false,
        },
        // no datumName
        {
            "NGVD29 height",
            R"X( { 
                "verticalCRS": {
                    "crsName": "NGVD29 height",
                    "epsg": 7968,
                    "type": "GEOID",
                    "description": "National Geodetic Vertical Datum 1929 - Meter",
                    "areaOfUse": "Continental United States(USA)",
                    "remarks": "Replaced by NAVD88 height.",
                    "units" : "meter",
                    "transforms": [
                        {
                            "target": "NAVD88 height",
                            "vertOffsetGrid": {
                                "direction": "Inverse",
                                "format": "VERTCON",
                                "files": [
                                    "./Usa/Vertcon/VERTCONC.94",
                                    "./Usa/Vertcon/VERTCONE.94",
                                    "./Usa/Vertcon/VERTCONW.94"
                                ]
                            }
                        }
                    ],
                    "extent": {
                        "southWest": {
                            "latitude": 14.51,
                            "longitude": 172.42
                        },
                        "northEast": {
                            "latitude": 71.4,
                            "longitude": -66.91
                        }
                    }
                }
            } )X",
            false,
        },
        // different offset
        {
            "Kiunga",
            R"X( { 
                "verticalCRS": {
                    "crsName": "Kiunga",
                    "epsg": 7652,
                    "type": "GEOID",
                    "description": "Kiunga Height",
                    "areaOfUse": "Papua New Guinea - onshore south of 5 degrees S and west of 144 degrees E",
                    "remarks": "Kiunga height = WGS 84 ellipsoid height - value of geoid undulation derived by bilinear interpolation of EGM2008 geoid model - 3.0m = EGM2008 height - 3.0m. See CRS code 3855 and transformation code 3858.",
                    "units" : "meter",
                    "transforms": [
                        {
                            "target": "EGM2008 height",
                            "verticalOffset": {
                                "offset": -2,
                                "units" : "meter"
                                }
                        }      
                    ],
                    "extent": {
                        "southWest": {
                            "latitude": -9.35,
                            "longitude": 140.85
                        },
                        "northEast": {
                            "latitude": -5,
                            "longitude": 144.01
                        }
                    }
                }
            } )X",
            false,
        }
    };


/*---------------------------------------------------------------------------------**//**
* Create a BaseGCS and set it's vertical datum from a name, create another BaseGCS and
* set it's vertical datum from a Json def, check to make sure they are equal.
* Assumes that the named item in the dictionary is equal to the Json fragment above,
* if the dictionary item changes this will fail.
* Also tests == operators and IsEqual()
* @bsimethod                                                    Sarah.Keenan  2024-10
+---------------+---------------+---------------+---------------+---------------+------*/
// SK TODO: rewrite this test to be more flexible as small dictionary changes cause it to fail
TEST_P(VerticalDatumSerializationTests, VerticalDatumSerializationTest1)
{
    verticalDatumSerializationTest theTestParam = GetParam(); 

    GeoCoordinates::BaseGCSPtr testGCS1 = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    StatusInt status = testGCS1->SetVerticalDatumFromName(theTestParam.m_name.c_str()); // set from named item in vertical datum dictionary
    ASSERT_TRUE(SUCCESS == status);

    GeoCoordinates::BaseGCSPtr testGCS2 = GeoCoordinates::BaseGCS::CreateGCS("LL84");    
    status = testGCS2->SetVerticalDatumFromJsonString(theTestParam.m_jsonStr); // set from Json string
    ASSERT_TRUE(SUCCESS == status);

    // both should be equal even though the vertical datum was set using different methods
    // (unless we are testing the non equal items in the list of tests)
    ASSERT_TRUE(theTestParam.m_equalToDictionaryItem == testGCS1->IsEqual(*(testGCS2.get())));

    Utf8String gcs1String, gcs2String;
    Json::Value valueOut(Json::objectValue);
    status = testGCS1->ToJson(valueOut);
    ASSERT_TRUE(SUCCESS == status);
    gcs1String = valueOut.toStyledString();

    Json::Value valueOut2(Json::objectValue);

    status = testGCS1->ToJson(valueOut2);
    ASSERT_TRUE(SUCCESS == status);
    gcs2String = valueOut2.toStyledString();

    // boths gcs were equal so should serialize exactly the same json string
    ASSERT_TRUE(0 == gcs1String.CompareTo(gcs2String));
}

/*---------------------------------------------------------------------------------**//**
* @bsi                                                          Sarah.Keenan   02/2025
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalDatumTransformsAvailableTest
{
    Utf8String  m_jsonStr;
    StatusInt   m_expectedStatus;
};

// Preparation of required environment
class VerticalDatumTransformsAvailableTests : public ::testing::TestWithParam< verticalDatumTransformsAvailableTest >
{   
public:
    virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
    virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

    VerticalDatumTransformsAvailableTests() {};
    ~VerticalDatumTransformsAvailableTests() {};
};

static bvector<verticalDatumTransformsAvailableTest> s_listOfTransformsAvailableTests = 
{
    // [equivalent Json definition] [status on setting vertical datum]
    // --> One transform, should be able to be initialized <---
    {
        R"X( { 
                "verticalCRS": {
                    "crsName": "Custom user height",
                    "datumName": "EGM96 geoid",
                    "epsg": 5773,
                    "type": "GEOID",
                    "description": "Earth Geoid Model 1996 with 15 seconds density",
                    "areaOfUse": "World",
                    "remarks": "Zero-height surface resulting from the application of the EGM96 geoid model to the WGS 84 ellipsoid. Replaces EGM84 height (CRS code 5798). Replaced by EGM2008 height (CRS code 3855).",
                    "units" : "meter",
                    "transforms": [
                        {
                            "target" : "WGS84",
                            "geoidSeparationGrid": {
                                "direction": "Direct",
                                "format": "GRD",
                                "files": [
                                    "./World/WW15MGH.GRD"
                                ]
                            }
                        }
                    ],
                    "extent": { "southWest": { "latitude": -90, "longitude": -180}, "northEast": { "latitude": 90.0, "longitude": 180.0 }}
                }
            } )X",
        SUCCESS,
    },
    // --> Two transforms, only one should be able to be initialized because of bad transform name <---
    {
        R"X( { 
                "verticalCRS": {
                    "crsName": "Custom user height",
                    "datumName": "EGM96 geoid",
                    "epsg": 5773,
                    "type": "GEOID",
                    "description": "Earth Geoid Model 1996 with 15 seconds density",
                    "areaOfUse": "World",
                    "remarks": "Zero-height surface resulting from the application of the EGM96 geoid model to the WGS 84 ellipsoid. Replaces EGM84 height (CRS code 5798). Replaced by EGM2008 height (CRS code 3855).",
                    "units" : "meter",
                    "transforms": [
                        {
                            "target" : "WGS84",
                            "geoidSeparationGrid": {
                                "direction": "Direct",
                                "format": "GRD",
                                "files": [
                                    "./World/WW15MGH.GRD"
                                ]
                            }
                        },
                        {
                            "target" : "WGS85",
                            "geoidSeparationGrid": {
                                "direction": "Direct",
                                "format": "GRD",
                                "files": [
                                    "./World/WW15MGH.GRD"
                                ]
                            }
                        }
                    ],
                    "extent": { "southWest": { "latitude": -90, "longitude": -180}, "northEast": { "latitude": 90.0, "longitude": 180.0 }}
                }
            } )X",
        GeoCoordinates::GEOCOORDERR_NotAllTransformsAvailable,
    }
};

/*---------------------------------------------------------------------------------**//**
* Create BaseGCS and set it's vertical datum from a Json def, tests for return value
* for SUCCESS or for the case where not all transforms can be initialized
* @bsimethod                                                    Sarah.Keenan  2025-02
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(VerticalDatumTransformsAvailableTests, VerticalDatumTransformsCanBeInitializedTest)
{
    verticalDatumTransformsAvailableTest theTestParam = GetParam(); 

    GeoCoordinates::BaseGCSPtr testGCS2 = GeoCoordinates::BaseGCS::CreateGCS("LL84");    
    StatusInt status = testGCS2->SetVerticalDatumFromJsonString(theTestParam.m_jsonStr); // set from Json string
    ASSERT_TRUE(status == theTestParam.m_expectedStatus);
}


/*---------------------------------------------------------------------------------**//**
* @bsi                                                   Sarah.Keenan   11/2024
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalDatumWKTTest
{
    GeoCoordinates::WKTOptionsFlags m_wktOptionsFlags;
    Utf8String     m_name;
    Utf8String     m_horizonalName;
    Utf8String     m_wktExpected;
};

// Preparation of required environment
class VerticalDatumWKTTests : public ::testing::TestWithParam< verticalDatumWKTTest >
{   
public:
    virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
    virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

    VerticalDatumWKTTests() {};
    ~VerticalDatumWKTTests() {};
};

static bvector<verticalDatumWKTTest> s_listOfWKTTests = 
{
    // Legacy vertical datum
    { GeoCoordinates::WKTOptionsFlags::DefaultOptions, "ELLIPSOID", "LL84", R"X(COMPD_CS["LL84",GEOGCS["LL84",DATUM["WGS84",SPHEROID["WGS84",6378137.000,298.25722293]],PRIMEM["Greenwich",0],UNIT["Degree",0.01745329251994]],VERT_CS["Ellipsoid Height",VERT_DATUM["Ellipsoid",2002],UNIT["Meters",1.000000]]])X", },
    { GeoCoordinates::WKTOptionsFlags::DefaultOptions, "GEOID", "LL84", R"X(COMPD_CS["LL84",GEOGCS["LL84",DATUM["WGS84",SPHEROID["WGS84",6378137.000,298.25722293]],PRIMEM["Greenwich",0],UNIT["Degree",0.01745329251994]],VERT_CS["Generic Geoid",VERT_DATUM["Generic Vertical Datum",2005],UNIT["Meters",1.000000]]])X", },

    // Uses full vertical datum name but should return old legacy code based name in WKT as the test is using GetCompoundCSWellKnownText() which 
    { GeoCoordinates::WKTOptionsFlags::DefaultOptions, "WGS84", "LL84", R"X(COMPD_CS["LL84",GEOGCS["LL84",DATUM["WGS84",SPHEROID["WGS84",6378137.000,298.25722293]],PRIMEM["Greenwich",0],UNIT["Degree",0.01745329251994]],VERT_CS["Ellipsoid Height",VERT_DATUM["Ellipsoid",2002],UNIT["Meters",1.000000]]])X", },
    { GeoCoordinates::WKTOptionsFlags::DefaultOptions, "NAVD88 height", "DE83", R"X(COMPD_CS["DE83",PROJCS["DE83",GEOGCS["NAD83",DATUM["NAD83",SPHEROID["GRS1980",6378137.000,298.25722210]],PRIMEM["Greenwich",0],UNIT["Degree",0.017453292519943295]],PROJECTION["Transverse Mercator"],PARAMETER["False Easting",200000.000],PARAMETER["False Northing",0.000],PARAMETER["Scale Reduction",0.999995000000],PARAMETER["Central Meridian",-75.41666666666667],PARAMETER["Origin Latitude",38.00000000000000],UNIT["Meter",1.00000000000000]],VERT_CS["NAVD88",VERT_DATUM["NAVD88",2005],UNIT["Meter",1.000000]]])X", },

    // Full vertical datum
    { GeoCoordinates::WKTOptionsFlags::FullVerticalDatumName, "WGS84", "LL84", R"X(COMPD_CS["LL84",GEOGCS["LL84",DATUM["WGS84",SPHEROID["WGS84",6378137.000,298.25722293]],PRIMEM["Greenwich",0],UNIT["Degree",0.01745329251994]],VERT_CS["WGS84",VERT_DATUM["WGS_1984",2002],UNIT["Meters",1.000000]]])X", },
    { GeoCoordinates::WKTOptionsFlags::FullVerticalDatumName, "EGM96 height", "LL84", R"X(COMPD_CS["LL84",GEOGCS["LL84",DATUM["WGS84",SPHEROID["WGS84",6378137.000,298.25722293]],PRIMEM["Greenwich",0],UNIT["Degree",0.01745329251994]],VERT_CS["EGM96 height",VERT_DATUM["EGM96 geoid",2005],UNIT["Meters",1.000000]]])X", },
    { GeoCoordinates::WKTOptionsFlags::FullVerticalDatumName, "NAVD88 height", "LL84", R"X(COMPD_CS["LL84",GEOGCS["LL84",DATUM["WGS84",SPHEROID["WGS84",6378137.000,298.25722293]],PRIMEM["Greenwich",0],UNIT["Degree",0.01745329251994]],VERT_CS["NAVD88 height",VERT_DATUM["North American Vertical Datum 1988",2005],UNIT["Meters",1.000000]]])X", },
    { GeoCoordinates::WKTOptionsFlags::FullVerticalDatumName, "NGVD29 height", "LL84", R"X(COMPD_CS["LL84",GEOGCS["LL84",DATUM["WGS84",SPHEROID["WGS84",6378137.000,298.25722293]],PRIMEM["Greenwich",0],UNIT["Degree",0.01745329251994]],VERT_CS["NGVD29 height",VERT_DATUM["National Geodetic Vertical Datum 1929",2005],UNIT["Meters",1.000000]]])X", },
    { GeoCoordinates::WKTOptionsFlags::FullVerticalDatumName, "Kiunga", "LL84", R"X(COMPD_CS["LL84",GEOGCS["LL84",DATUM["WGS84",SPHEROID["WGS84",6378137.000,298.25722293]],PRIMEM["Greenwich",0],UNIT["Degree",0.01745329251994]],VERT_CS["Kiunga",VERT_DATUM["Kiunga",2005],UNIT["Meters",1.000000]]])X", },
    { GeoCoordinates::WKTOptionsFlags::FullVerticalDatumName, "NAVD88(Geoid12b) height", "LL84", R"X(COMPD_CS["LL84",GEOGCS["LL84",DATUM["WGS84",SPHEROID["WGS84",6378137.000,298.25722293]],PRIMEM["Greenwich",0],UNIT["Degree",0.01745329251994]],VERT_CS["NAVD88(Geoid12b) height",VERT_DATUM["North American Vertical Datum 1988",2005],UNIT["Meters",1.000000]]])X", },
};

TEST_P(VerticalDatumWKTTests, VerticalDatumWKTTest1)
{
    verticalDatumWKTTest theTestParam = GetParam(); 

    GeoCoordinates::BaseGCSPtr testGCS1 = GeoCoordinates::BaseGCS::CreateGCS(theTestParam.m_horizonalName.c_str());
    ASSERT_TRUE(testGCS1.IsValid() && testGCS1->IsValid());
    StatusInt status = testGCS1->SetVerticalDatumFromName(theTestParam.m_name.c_str()); // try to set from named item in vertical datum dictionary
    if (GeoCoordinates::GEOCOORDERR_CoordSysNotFound == status)
        status = testGCS1->SetVerticalDatumByKey(theTestParam.m_name.c_str()); // not found in the dictionary, try as key name (i.e. legacy code)
    ASSERT_TRUE(SUCCESS == status);

    Utf8String wkt;
    ASSERT_TRUE(SUCCESS == testGCS1->GetCompoundCSWellKnownText(wkt, GeoCoordinates::BaseGCS::wktFlavorAutodesk, theTestParam.m_wktOptionsFlags));
    ASSERT_TRUE(0 == wkt.CompareToI(theTestParam.m_wktExpected));

    GeoCoordinates::BaseGCSPtr testGCS2 = GeoCoordinates::BaseGCS::CreateGCS();
    ASSERT_TRUE(SUCCESS == testGCS2->GeoCoordinates::BaseGCS::InitFromWellKnownText(nullptr, nullptr, GeoCoordinates::BaseGCS::wktFlavorUnknown, wkt.c_str()));
    ASSERT_TRUE(testGCS1->IsEquivalent(*testGCS2));
}

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumSerializationTests,
                         ValuesIn(s_listOfTests));

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumTransformsAvailableTests,
                         ValuesIn(s_listOfTransformsAvailableTests));

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumWKTTests,
                         ValuesIn(s_listOfWKTTests));
