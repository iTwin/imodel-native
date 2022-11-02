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
class GCSGeneralWKTRepresentativeTests : public ::testing::TestWithParam< Utf8String >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        GCSGeneralWKTRepresentativeTests() {};
        ~GCSGeneralWKTRepresentativeTests() {};

    };


        
/*---------------------------------------------------------------------------------**//**
* WKT Testing
* The purpose of this test is to create a WKT from every dictionary entry both
* as a plain WKT and as a compound WKT and try to recreate an equivalent 
* geographical coordinate system and this for every flavor supported.
* Also we will use the original flavor once then the unknown flavor that is supposed to
* attempt to determine the flavor upon analysis of the WKT string
* NOTE: Not all entries will pass this test.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P (GCSGeneralWKTRepresentativeTests, WKTGenerateThenParseTest)
    {
    // We do not check the OSGB GCS since usually those are not available because of the
    // abssense of the OSGB grid files.
    const Utf8CP keyname = GetParam().c_str();
    Utf8String theKeyname(keyname);

    if (theKeyname.CompareTo("OSGB-GPS-2002") == 0 ||
        theKeyname.CompareTo("OSGB-GPS-2015") == 0 ||
        theKeyname.CompareTo("OSGB-GPS-1997") == 0)
        return;

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(keyname);
    
    // Check transformation properties
    ASSERT_TRUE(!toto.IsNull());
    
    ASSERT_TRUE(toto.IsValid());

    auto projectionCode = toto->GetProjectionCode();

    // The following set cannot be converted to any WKT flavor
    if (projectionCode == GeoCoordinates::BaseGCS::pcvAzimuthalEquidistantElevatedEllipsoid   ||
        projectionCode == GeoCoordinates::BaseGCS::pcvModifiedStereographic   ||
        projectionCode == GeoCoordinates::BaseGCS::pcvBipolarObliqueConformalConic   ||
        projectionCode == GeoCoordinates::BaseGCS::pcvHotineObliqueMercator1UV  ||
        projectionCode == GeoCoordinates::BaseGCS::pcvHotineObliqueMercator2UV ||
        projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys3401 ||
        projectionCode == GeoCoordinates::BaseGCS::pcvObliqueMercatorMinnesota)
        return;

        // These should work in one flavor but they do not (CSMAP issue)
    if (/* (currentFlavor != GeoCoordinates::BaseGCS::wktFlavorAutodesk) && */
         (projectionCode == GeoCoordinates::BaseGCS::pcvModifiedStereographic    ||
          projectionCode == GeoCoordinates::BaseGCS::pcvSnyderObliqueStereographic    ||
          projectionCode == GeoCoordinates::BaseGCS::pcvCzechKrovak95))
        return;

    bvector<GeoCoordinates::BaseGCS::WktFlavor> listOfWKTFlavors;
    
    listOfWKTFlavors.push_back(GeoCoordinates::BaseGCS::wktFlavorOGC);
    listOfWKTFlavors.push_back(GeoCoordinates::BaseGCS::wktFlavorGeoTiff);
    listOfWKTFlavors.push_back(GeoCoordinates::BaseGCS::wktFlavorESRI);
    listOfWKTFlavors.push_back(GeoCoordinates::BaseGCS::wktFlavorOracle);
    listOfWKTFlavors.push_back(GeoCoordinates::BaseGCS::wktFlavorGeoTools);
    listOfWKTFlavors.push_back(GeoCoordinates::BaseGCS::wktFlavorEPSG);
    listOfWKTFlavors.push_back(GeoCoordinates::BaseGCS::wktFlavorOracle9);
    listOfWKTFlavors.push_back(GeoCoordinates::BaseGCS::wktFlavorAutodesk);
    
    for (auto currentFlavor : listOfWKTFlavors)
        {
        // Some flavors are incompatible with projection methods

        if ((currentFlavor == GeoCoordinates::BaseGCS::wktFlavorOracle) && 
            (projectionCode == GeoCoordinates::BaseGCS::pcvSouthOrientedTransverseMercator  ||
             projectionCode == GeoCoordinates::BaseGCS::pcvRectifiedSkewOrthomorphicCentered ||
             projectionCode == GeoCoordinates::BaseGCS::pcvRectifiedSkewOrthomorphicOrigin ||
             projectionCode == GeoCoordinates::BaseGCS::pcvLambertConformalConicWisconsin  ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorWisconsin  ||
             projectionCode == GeoCoordinates::BaseGCS::pcvLambertConformalConicMinnesota ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorMinnesota ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorOstn97 ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorOstn02 ||
             projectionCode == GeoCoordinates::BaseGCS::pcvObliqueCylindricalHungary))
            continue;

        if ((currentFlavor == GeoCoordinates::BaseGCS::wktFlavorESRI) && 
            (projectionCode == GeoCoordinates::BaseGCS::pcvSouthOrientedTransverseMercator    ||
             projectionCode == GeoCoordinates::BaseGCS::pcvLambertConformalConicWisconsin    ||
             projectionCode == GeoCoordinates::BaseGCS::pcvRectifiedSkewOrthomorphicOrigin   ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorWisconsin    ||
             projectionCode == GeoCoordinates::BaseGCS::pcvLambertConformalConicMinnesota   ||
             projectionCode == GeoCoordinates::BaseGCS::pcvRectifiedSkewOrthomorphicCentered   ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorMinnesota   ||
             projectionCode == GeoCoordinates::BaseGCS::pcvObliqueCylindricalHungary   ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys34    ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys3499 ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorOstn97   ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorOstn02   ||
             projectionCode == GeoCoordinates::BaseGCS::pcvSnyderTransverseMercator    ||
             projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorKruger))
            continue;

        // WKT Validation
        // We expect the WKT creation to be performed non empty and be able to recreate an equivalent coordinate system.
        Utf8String WKT;
    
        ASSERT_TRUE(toto->GetWellKnownText(WKT, currentFlavor, true) == SUCCESS);
    
        ASSERT_TRUE(WKT.size() > 0);

        // Now we recreate the GCS
        GeoCoordinates::BaseGCSPtr recreatedGCS = GeoCoordinates::BaseGCS::CreateGCS();
    
        ASSERT_TRUE(!recreatedGCS.IsNull());
    
        if (!recreatedGCS.IsNull())
            EXPECT_TRUE(GeoCoordinates::GeoCoordParse_Success == recreatedGCS->InitFromWellKnownText(NULL, NULL, WKT.c_str()));
        }
    }

    
INSTANTIATE_TEST_SUITE_P(GCSGeneralWKTRepresentativeTests_Combined,
                        GCSGeneralWKTRepresentativeTests,
                        ValuesIn(GeoCoordTestCommon::GetRepresentativeListOfGCS()));




