//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/DictionaryTests/GCSUnaryTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>

#include <GeoCoord/BaseGeoCoord.h>
#include <GeoCoord/GCSLibrary.h>
#include "GCSUnaryTester.h"

using namespace ::testing;

GCSUnaryTester::GCSUnaryTester() 
    {
    //static bool initialised = false;

    //if (!initialised)
    //    GeoCoordinates::BaseGCS::Initialize(L".//Assets//DgnGeoCoord");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Alain.Robert  02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<WString> const& s_GetListOfCoordinateSystems ()
    {
    static bvector<WString> listOfCoordinateSystems;

    if(listOfCoordinateSystems.empty())
        {
        GeoCoordinates::BaseGCS::Initialize(L".//Assets//DgnGeoCoord");
        
        size_t numCS = GeoCoordinates::LibraryManager::Instance()->GetSystemLibrary()->GetCSCount();
        for (size_t indexCS = 0 ; indexCS < numCS; indexCS++)
            {
            WString csName;
            GeoCoordinates::LibraryManager::Instance()->GetSystemLibrary()->GetCSName((uint32_t)indexCS, csName);
            listOfCoordinateSystems.push_back(csName);
            }
        }

    return listOfCoordinateSystems;
    }

    
//==================================================================================
// Domain
//==================================================================================
TEST_P (GCSUnaryTester, InstantiationTest)
    {
    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(GetParam().c_str());
    
    // Check transformation properties
    EXPECT_TRUE(!toto.IsNull());
    
    EXPECT_TRUE(toto->IsValid());
    }
   
#if (0)
//==================================================================================
// Domain
//==================================================================================
TEST_P (GCSUnaryTester, UserDomainTest)
    {

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(GetParam().c_str());

    
    double minUsefulLongitude = toto->GetMinimumUsefulLongitude();            
    double maxUsefulLongitude = toto->GetMaximumUsefulLongitude();
    double minUsefulLatitude = toto->GetMinimumUsefulLatitude();
    double maxUsefulLatitude = toto->GetMaximumUsefulLatitude();
    
    double minLongitude = toto->GetMinimumLongitude();
    double maxLongitude = toto->GetMaximumLongitude();
    double minLatitude = toto->GetMinimumLatitude();
    double maxLatitude = toto->GetMaximumLatitude();
        
    if (toto->GetProjectionCode() == GeoCoordinates::BaseGCS::pcvUnity)
        {
        // Lat/long based store user domain differently
        EXPECT_TRUE(minLongitude < maxLongitude);
        EXPECT_TRUE(minLatitude < maxLatitude);
        }
    else
        {
        EXPECT_TRUE(minLongitude < maxLongitude);
        EXPECT_TRUE(minLatitude < maxLatitude);
        
        EXPECT_TRUE(minUsefulLongitude < maxUsefulLongitude);
        EXPECT_TRUE(minUsefulLatitude < maxUsefulLatitude);
        
        EXPECT_TRUE(minLatitude == minUsefulLatitude);
        EXPECT_TRUE(maxLatitude == maxUsefulLatitude);
        EXPECT_TRUE(minLongitude == minUsefulLongitude);
        EXPECT_TRUE(maxLongitude == maxUsefulLongitude);
        }

    // Check transformation properties
    ASSERT_TRUE(true);
    }

#endif
    
#if (0)
    
//==================================================================================
// WKT Testing
// The purpose of this test is to create a WKT from every dictionary entry both
// as a plain WKT and as a compound WKT and try to recreate an equivalent 
// geographical coordinate system and this for every flavor supported.
// Also we will use the original flavor once then the unknown flavor that is supposed to
// attempt to determine the flavor upon analysis of the WKT string
// NOTE: Not all entries will pass this test.
//==================================================================================
TEST_P (GCSUnaryTester, WKTTest)
    {

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(GetParam().c_str());
    
    // Check transformation properties
    EXPECT_TRUE(!toto.IsNull());
    
    EXPECT_TRUE(toto.IsValid());

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
        // WKT Validation
        // We expect the WKT creation to be performed non empty and be able to recreate an equivalent coordinate system.
        WString WKT;
    
        EXPECT_TRUE(toto->GetWellKnownText(WKT, currentFlavor, true) == SUCCESS);
    
        EXPECT_TRUE(WKT.size() > 0);
    
        GeoCoordinates::BaseGCSPtr recreatedGCS = GeoCoordinates::BaseGCS::CreateGCS();
    
        EXPECT_TRUE(!recreatedGCS.IsNull());
    
        if (recreatedGCS.IsValid())
            {
            EXPECT_TRUE(recreatedGCS->InitFromWellKnownText(NULL, NULL, currentFlavor, WKT.c_str()));
            
            EXPECT_TRUE(recreatedGCS->IsEquivalent(*toto));
            }
    
        WString compoundWKT;
    
        EXPECT_TRUE(toto->GetCompoundCSWellKnownText(compoundWKT, currentFlavor, false));
    
        EXPECT_TRUE(compoundWKT.size() > 0);
    
        recreatedGCS = GeoCoordinates::BaseGCS::CreateGCS();
    
        
        EXPECT_TRUE(!recreatedGCS.IsNull());
    
        if (recreatedGCS->IsValid())
            {
            EXPECT_TRUE(SUCCESS == recreatedGCS->InitFromWellKnownText(NULL, NULL, currentFlavor, compoundWKT.c_str()));
                
            EXPECT_TRUE(recreatedGCS->IsEquivalent(*toto));
            }
        }
    }
    
#endif
    
INSTANTIATE_TEST_CASE_P(GCSUnaryTester_Combined,
                        GCSUnaryTester,
                        ValuesIn(s_GetListOfCoordinateSystems ()));


#if (0)
                            // Scrap for later use
                            switch(toto->GetProjectionCode())
        {
        case pcvInvalid:
            EXPECT_TRUE(false);
            break;
            
            
        case pcvUnity:
            
            break;
            
        case pcvTransverseMercator:
            EXPECT_TRUE(SUCCESS == toto->SetFalseEasting(0.0));
            break;
            
        case pcvAlbersEqualArea:
            break;
            
        case pcvHotineObliqueMercator:
            break;
            
        case pcvMercator:
            break;
            
        case pcvLambertEquidistantAzimuthal:
            break;
            
        case pcvLambertTangential:
            break;
            
        case pcvAmericanPolyconic:
            break;
            
        case pcvModifiedPolyconic:                           
            break;
            
        case pcvLambertEqualAreaAzimuthal:                   
            break;
            
        case pcvEquidistantConic:                            
            break;
            
        case pcvMillerCylindrical:                           
            break;
            
        case pcvModifiedStereographic:                       
            break;
            
        case pcvNewZealandNationalGrid:                      
            break;
            
        case pcvSinusoidal:                                  
            break;
            
        case pcvOrthographic:                                
            break;
            
        case pcvGnomonic:                                    
            break;
            
        case pcvEquidistantCylindrical:                      
            break;
            
        case pcvVanderGrinten:                               
            break;
            
        case pcvCassini:                                     
            break;
            
        case pcvRobinsonCylindrical:                         
            break;
            
        case pcvBonne:                                       
            break;
            
        case pcvEckertIV:                                    
            break;
            
        case pcvEckertVI:                                    
            break;
            
        case pcvMollweide:                                   
            break;
            
        case pcvGoodeHomolosine:                             
            break;
            
        case pcvEqualAreaAuthalicNormal:                     
            break;
            
        case pcvEqualAreaAuthalicTransverse:                 
            break;
            
        case pcvBipolarObliqueConformalConic:                
            break;
            
        case pcvObliqueCylindricalSwiss:                     
            break;
            
        case pcvPolarStereographic:                          
            break;
            
        case pcvObliqueStereographic:                        
            break;
            
        case pcvSnyderObliqueStereographic:                  
            break;
            
        case pcvLambertConformalConicOneParallel:            
            break;
            
        case pcvLambertConformalConicTwoParallel:            
            break;
            
        case pcvLambertConformalConicBelgian:                
            break;
            
        case pcvLambertConformalConicWisconsin:              
            break;
            
        case pcvTransverseMercatorWisconsin:                 
            break;
            
        case pcvLambertConformalConicMinnesota:              
            break;
            
        case pcvTransverseMercatorMinnesota:                 
            break;
            
        case pcvSouthOrientedTransverseMercator:             
            break;
            
        case pcvUniversalTransverseMercator:                 
            break;
            
        case pcvSnyderTransverseMercator:                    
            break;
            
        case pcvGaussKrugerTranverseMercator:                
            break;
            
        case pcvCzechKrovak:                                 
            break;
            
        case pcvCzechKrovakObsolete:                         
            break;
            
        case pcvMercatorScaleReduction:                      
            break;
            
        case pcvObliqueConformalConic:                       
            break;
            
        case pcvCzechKrovak95:                               
            break;
            
        case pcvCzechKrovak95Obsolete:                       
            break;
            
        case pcvPolarStereographicStandardLatitude:          
            break;
            
        case pcvTransverseMercatorAffinePostProcess:         
            break;
            
        case pcvNonEarth:                                    
            break;

        case pcvObliqueCylindricalHungary:                   
            break;
            
        case pcvTransverseMercatorDenmarkSys34:              
            break;
            
        case pcvTransverseMercatorOstn97:                    
            break;
            
        case pcvAzimuthalEquidistantElevatedEllipsoid:       
            break;
            
        case pcvTransverseMercatorOstn02:                    
            break;
            
        case pcvTransverseMercatorDenmarkSys3499:            
            break;
            
        case pcvTransverseMercatorKruger:                    
            break;
            
        case pcvWinkelTripel:                                
            break;
            
        case pcvNonEarthScaleRotation:                       
            break;
            
        case pcvLambertConformalConicAffinePostProcess:      
            break;
            
        case pcvTransverseMercatorDenmarkSys3401:            
            break;
            
        case pcvEquidistantCylindricalEllipsoid:             
            break;
            
        case pcvPlateCarree:                                 
            break;
            
        case pcvPopularVisualizationPseudoMercator:          
            break;
            
        case pcvHotineObliqueMercator1UV:                    
            break;
            
        case pcvHotineObliqueMercator1XY:                    
            break;
            
        case pcvHotineObliqueMercator2UV:                    
            break;
            
        case pcvHotineObliqueMercator2XY:                    
            break;
            
        case pcvRectifiedSkewOrthomorphic:                   
            break;
            
        case pcvRectifiedSkewOrthomorphicCentered:           
            break;
            
        case pcvRectifiedSkewOrthomorphicOrigin:             
            break;
            
        case pcvTotalUniversalTransverseMercator:            
            break;
            
        case pcvTotalTransverseMercatorBF:                   
            break;
            
        case pcvObliqueMercatorMinnesota:                    
            break;
            
        }
        
        #endif
