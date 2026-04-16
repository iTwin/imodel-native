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
struct dictionaryJsonTest
    {
    Utf8String     m_name;
    Utf8String     m_jsonDictionaryDef;
    StatusInt   m_testStatus;
    };
 
// Preparation of required environment
class VerticalDatumDictionaryTests : public ::testing::TestWithParam< dictionaryJsonTest >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        VerticalDatumDictionaryTests() {};
        ~VerticalDatumDictionaryTests() {};
    };

static bvector<dictionaryJsonTest> s_listOfDictionaryJsonTests = 
    {
        // vertica datum name, JSON dictionary fragment, expected result
        { 
            "",
            "", 
            GeoCoordinates::GEOCOORDERR_BadArg,
        },
        { 
            "GarbageIn",
            "{ garbageIn: \"garbageOut\" }",
            GeoCoordinates::GeoCoordParseStatus::GeoCoordParse_ParseError,
        },
        { 
            "NoVersion",
            R"X( {  "definitions" : [] 
                } )X",
            GeoCoordinates::GEOCOORDERR_NoVersion,
        },
        { 
            "UnsupportedVersion",
            R"X( {  "version" : 1.1,
                    "definitions" : [] 
                } )X",
            GeoCoordinates::GEOCOORDERR_UnsupportedVersion,
        },
        { 
            "TestCRSValid",
            R"X( {  "version" : 1.0,
                    "definitions" : [
                    {
                    "verticalCRS": {
                            "crsName": "TestCRSValid",
                            "epsg": 1234,
                            "type": "GEOID",
                            "description": "Test Height",
                            "areaOfUse": "Nowhere",
                            "remarks": "Only for testing",
                            "units" : "meter",
                            "transforms": [
                                {
                                    "target": "EGM96 height",
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
                        }
                    ]
                } )X",
            SUCCESS,
        },
        { 
            "TestCRSNoExtent",
            R"X( {  "version" : 1.0,
                    "definitions" : [
                    {
                    "verticalCRS": {
                            "crsName": "TestCRSNoExtent",
                            "epsg": 1234,
                            "type": "GEOID",
                            "description": "Test Height",
                            "areaOfUse": "Nowhere",
                            "remarks": "Only for testing",
                            "units" : "meter",
                            "transforms": [
                                {
                                    "target": "EGM2008",
                                        "verticalOffset": {
                                        "offset": -3,
                                            "units" : "meter"
                                    }
                                }      
                            ]
                            }
                        }
                    ]
                } )X",
            GeoCoordinates::GEOCOORDERR_CoordinateRange,
        },
        { 
            "TestCRSNoTransformTarget",
            R"X( {  "version" : 1.0,
                    "definitions" : [
                    {
                    "verticalCRS": {
                            "crsName": "TestCRSNoTransformTarget",
                            "datumName": "TestDatum",
                            "epsg": 1234,
                            "type": "GEOID",
                            "description": "Test Height",
                            "areaOfUse": "Nowhere",
                            "remarks": "Only for testing",
                            "units" : "meter",
                            "transforms": [
                                {}
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
                        }
                    ]
                } )X",
            GeoCoordinates::GEOCOORDERR_NoTransforms,
        },
        { 
            "TestCRSBadType",
            R"X( {  "version" : 1.0,
                    "definitions" : [
                    {
                    "verticalCRS": {
                        "id": "TestCRSBadType",
                            "epsg": 1234,
                            "type": "BADTYPE",
                            "description": "Test Height",
                            "areaOfUse": "Nowhere",
                            "remarks": "Only for testing",
                            "units" : "meter"
                            }
                        }
                    ]
                } )X",
            GeoCoordinates::GEOCOORDERR_UnknownDatumType,
        },
    };


/*---------------------------------------------------------------------------------**//**
* Query GeoCoord to find Vertical Datums that are availble for a specific position
* lat/long
* @bsimethod                                                    Sarah.Keenan  2024-07
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(VerticalDatumDictionaryTests, VerticalDatumDictionaryJsonTests)
{
    dictionaryJsonTest theTestParam = GetParam(); 

    StatusInt status = GeoCoordinates::BaseGCS::AddVerticalDatumsFromJsonString(theTestParam.m_jsonDictionaryDef);
    ASSERT_TRUE(theTestParam.m_testStatus == status);

    if (SUCCESS == status)
    {
        GeoCoordinates::BaseGCSPtr testGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");
        status = testGCS->SetVerticalDatumFromName(theTestParam.m_name.c_str());
        ASSERT_TRUE(SUCCESS == status);

        Utf8String name;
        testGCS->GetFullVerticalDatumName(name);
        ASSERT_TRUE(SUCCESS == status);
        ASSERT_TRUE(0 == name.CompareTo(theTestParam.m_name));
    }
}

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumDictionaryTests,
                         ValuesIn(s_listOfDictionaryJsonTests));
