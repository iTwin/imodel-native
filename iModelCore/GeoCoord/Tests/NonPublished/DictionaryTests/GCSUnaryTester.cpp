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
static bool s_AngleInDegreesEqual (double angle1, double angle2, double tolerance)
    {
    // Make sure tolerance is positive
    if (tolerance < 0.0)
        tolerance = -tolerance;

    if (angle1 - tolerance < angle2 && angle1 + tolerance > angle2)
        return true;
    
    // Even if the values are not equat they may still be equal angular-wise
    // Normalise the values
    while (angle1 < -180.0)
        angle1 += 360.0;

    while (angle1 > 180.0)
        angle1 -= 360.0;

    while (angle2 < -180.0)
        angle2 += 360.0;

    while (angle2 > 180.0)
        angle2 -= 360.0;


    if (angle1 - tolerance < angle2 && angle1 + tolerance > angle2)
        return true;

    return false;
    }    



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Alain.Robert  02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<WString> const& s_GetListOfCoordinateSystems ()
    {
    static bvector<WString> listOfCoordinateSystems;

    if(listOfCoordinateSystems.empty())
        {
        BeTest::Host& host = BeTest::GetHost();

        BeFileName path;
        host.GetDgnPlatformAssetsDirectory(path);

        path.AppendToPath (L"DgnGeoCoord");

        GeoCoordinates::BaseGCS::Initialize(path.c_str());
        
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
// Basic instanciation tests
//==================================================================================
TEST_P (GCSUnaryTester, InstantiationTest)
    {
    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(GetParam().c_str());
    
    // Check transformation properties
    EXPECT_TRUE(!toto.IsNull());
    
    // The two OSGB GCS may not be valid since they will only be properly created in the present co the OSGB specific grid shift files
    const WChar* keyname = toto->GetName();
    WString theKeyname(keyname);
    if (theKeyname.CompareTo(L"OSGB-GPS-2002") == 0 || theKeyname.CompareTo(L"OSGB-GPS-1997"))
        return;
        
    EXPECT_TRUE(toto->IsValid());
    }
   

//==================================================================================
// Domain
//==================================================================================
TEST_P (GCSUnaryTester, UserDomainTest)
    {

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(GetParam().c_str());

    const WChar* keyname = toto->GetName();
    WString theKeyname(keyname);
    if (theKeyname.CompareTo(L"OSGB-GPS-2002") == 0 || theKeyname.CompareTo(L"OSGB-GPS-1997"))
        return;
    
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
#if (0)
        // Lat/long based store user domain differently
        EXPECT_TRUE(minLongitude < maxLongitude) << GetParam().c_str();
        EXPECT_TRUE(minLatitude < maxLatitude) << GetParam().c_str();
#endif
        }
    else
        {
        EXPECT_TRUE(minLongitude < maxLongitude) << GetParam().c_str();
        EXPECT_TRUE(minLatitude < maxLatitude) << GetParam().c_str();
        
        // Notice that we do not check that the minimum useful longitude is smaller than the maximum useful longitude.
        // This is intentional since sometimes the computations make it that the numerically the minimum can be greater than
        // the maximum when the range crosses the -180 limit. Since these must be equal (angularly speaking) with 
        // set domain and these are checked then the condition is effectively checked below.

        EXPECT_TRUE(s_AngleInDegreesEqual(minLatitude, minUsefulLatitude, 0.0000001)) << GetParam().c_str();
        EXPECT_TRUE(s_AngleInDegreesEqual(maxLatitude, maxUsefulLatitude, 0.0000001)) << GetParam().c_str();
        EXPECT_TRUE(s_AngleInDegreesEqual(minLongitude, minUsefulLongitude, 0.0000001)) << GetParam().c_str();
        EXPECT_TRUE(s_AngleInDegreesEqual(maxLongitude, maxUsefulLongitude, 0.0000001)) << GetParam().c_str();
        }

    }

    
    
//==================================================================================
// WKT Testing
// The purpose of this test is to create a WKT from every dictionary entry both
// as a plain WKT and as a compound WKT and try to recreate an equivalent 
// geographical coordinate system and this for every flavor supported.
// Also we will use the original flavor once then the unknown flavor that is supposed to
// attempt to determine the flavor upon analysis of the WKT string
// NOTE: Not all entries will pass this test.
//==================================================================================
TEST_P (GCSUnaryTester, WKTGenerationTest)
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

        WString compoundWKT;
    
        EXPECT_TRUE(toto->GetCompoundCSWellKnownText(compoundWKT, currentFlavor, false));
    
        EXPECT_TRUE(compoundWKT.size() > 0);

        }
    }
    
    
INSTANTIATE_TEST_CASE_P(GCSUnaryTester_Combined,
                        GCSUnaryTester,
                        ValuesIn(s_GetListOfCoordinateSystems ()));



        

//==================================================================================
// WKT Testing
// The purpose of this test is to create a WKT from every dictionary entry both
// as a plain WKT and as a compound WKT and try to recreate an equivalent 
// geographical coordinate system and this for every flavor supported.
// Also we will use the original flavor once then the unknown flavor that is supposed to
// attempt to determine the flavor upon analysis of the WKT string
// NOTE: Not all entries will pass this test.
//==================================================================================
TEST_P (GCSUnaryTester, WKTParseTest)
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
#if (1)
        GeoCoordinates::BaseGCSPtr recreatedGCS = GeoCoordinates::BaseGCS::CreateGCS();
    
        EXPECT_TRUE(!recreatedGCS.IsNull());
    
        if (recreatedGCS.IsValid())
            {
            EXPECT_TRUE(recreatedGCS->InitFromWellKnownText(NULL, NULL, currentFlavor, WKT.c_str()));
            
            EXPECT_TRUE(recreatedGCS->IsEquivalent(*toto));
            }
#endif
        WString compoundWKT;
    
        EXPECT_TRUE(toto->GetCompoundCSWellKnownText(compoundWKT, currentFlavor, false));
    
        EXPECT_TRUE(compoundWKT.size() > 0);
#if (1)
        recreatedGCS = GeoCoordinates::BaseGCS::CreateGCS();
    
        
        EXPECT_TRUE(!recreatedGCS.IsNull());
    
        if (recreatedGCS->IsValid())
            {
            EXPECT_TRUE(SUCCESS == recreatedGCS->InitFromWellKnownText(NULL, NULL, currentFlavor, compoundWKT.c_str()));
                
            EXPECT_TRUE(recreatedGCS->IsEquivalent(*toto));
            }
#endif
        }
    }
    
#endif
    

#endif
