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

    m_LL84GCS = GeoCoordinates::BaseGCS::CreateGCS(L"LL84");

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

bool    bsiDPoint2d_isPointInConvexPolygonFixed
(
DPoint2dCP pPoint,
DPoint2dCP pPointArray,
int             numPoint,
int             sense
)

    {
    double areaFactor = (double)sense;
    double dot;
    int i1, i0;

    if (numPoint < 3)
        return  false;
    if (sense == 0)
        areaFactor = bsiDPoint2d_getPolygonArea (pPointArray, numPoint);

    for (i0 = numPoint - 1, i1 = 0; i1 < numPoint; i0 = i1++)
        {
        dot = pPoint->CrossProductToPoints (pPointArray[i0], pPointArray[i1]);
        if (dot * areaFactor < 0.0)
            return false;
        }
    return true;
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
// UserDomainTest
//==================================================================================
TEST_P (GCSUnaryTester, UserDomainTest)
    {

    // We do not check the OSGB GCS since usually those are not available because of the
    // abscense of the 
    const WChar* keyname = GetParam().c_str();
    WString theKeyname(keyname);
    if (theKeyname.CompareTo(L"OSGB-GPS-2002") == 0 || theKeyname.CompareTo(L"OSGB-GPS-1997") == 0)
        return;

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(keyname);
    
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
// UserDomainCartesianConversionTest
// This test makes sure that no error is reported when converting from a lat/long
// pair reported in the user domain to a cartesian coordinate.
// The only exception to this rule is the Danish coordinate system
// that have highly complex mathematical domain that are fully inscribed in the
// use domain extent still not convering the corners.
// Notice that conversion from lat/long in the GCS datum to cartesian only is performed
// so we do not test as a side line the conversion of the datums.
//==================================================================================
TEST_P (GCSUnaryTester, UserDomainConvertionTest)
    {

    // We do not check the OSGB GCS since usually those are not available because of the
    // abscense of the 
    const WChar* keyname = GetParam().c_str();
    WString theKeyname(keyname);
    if (theKeyname.CompareTo(L"OSGB-GPS-2002") == 0 || theKeyname.CompareTo(L"OSGB-GPS-1997") == 0)
        return;

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(keyname);

    double minUsefulLongitude = toto->GetMinimumUsefulLongitude();            
    double maxUsefulLongitude = toto->GetMaximumUsefulLongitude();
    double minUsefulLatitude = toto->GetMinimumUsefulLatitude();
    double maxUsefulLatitude = toto->GetMaximumUsefulLatitude();
    
      
    // Danish coordinate systems cannot be validated since their mathematical domain is smaller (and not square) to their user domain
    auto projectionCode = toto->GetProjectionCode();

    if (projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys34  ||
        projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys3499 ||
        projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys3401)
        return;

    
    
    if (projectionCode == GeoCoordinates::BaseGCS::pcvUnity)
        {
#if (0)
        // Lat/long based store user domain differently we will deal later.
        EXPECT_TRUE(minLongitude < maxLongitude) << GetParam().c_str();
        EXPECT_TRUE(minLatitude < maxLatitude) << GetParam().c_str();
#endif
        }
    else
        {
        double latitudeStep = (maxUsefulLatitude - minUsefulLatitude) / 10.1;
        double longitudeStep = (maxUsefulLongitude - minUsefulLongitude) / 10.1;
        // Note that the tolerance added serves to prevent user domain error that may result from floating point rounding errors
        // as some projection methods validate on the exact same value we use but using a different computational path
        // that injects different rounding errors. The tolerance is 0.001 second of arc (or a max of 3 centimeters)
        for (double latitude = minUsefulLatitude + 0.0000028; latitude < maxUsefulLatitude - 0.0000028 ; latitude += latitudeStep)
            {
            for (double longitude = minUsefulLongitude + 0.0000028 ; longitude < maxUsefulLongitude - 0.0000028; longitude += longitudeStep)
                {
                GeoPoint latLongPoint;
                latLongPoint.longitude = longitude;
                latLongPoint.latitude = latitude;
                latLongPoint.elevation = 0.0;
                DPoint3d outPoint;
                
                // Even though the error is not fatal we stop since all it does is clutter the output in case of multiple errors.
                ASSERT_TRUE(REPROJECT_Success == toto->CartesianFromLatLong(outPoint, latLongPoint));
                }
            }
        }

    }
    
//==================================================================================
// MathematicalDomainCartesianConversionTest
// This test makes sure that no error is reported when converting from a lat/long
// pair reported in the mathematical domain to a cartesian coordinate.
// Notice that conversion from lat/long in the GCS datum to cartesian only is performed
// so we do not test as a side line the conversion of the datums.
//==================================================================================
TEST_P (GCSUnaryTester, MathematicalDomainConvertionTest)
    {

    // We do not check the OSGB GCS since usually those are not available because of the
    // abscense of the 
    const WChar* keyname = GetParam().c_str();
    WString theKeyname(keyname);
    if (theKeyname.CompareTo(L"OSGB-GPS-2002") == 0 || theKeyname.CompareTo(L"OSGB-GPS-1997") == 0)
        return;

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(keyname);

    // Obtain the mathematical domain of the GCS
    bvector<GeoPoint> shape;
    ASSERT_TRUE(SUCCESS == toto->GetMathematicalDomain(shape));

    ASSERT_TRUE(shape.size() > 0);

    // Copy GeoPoints to plain points (for shape computations)
    // Compute extent at the same time for test.
    double minX = shape[0].longitude;
    double minY = shape[0].latitude;
    double maxX = minX;
    double maxY = minY;

    bvector<DPoint2d> cartesianShape;
    for (auto currentGeoPoint: shape)
        {
        minX = MIN(minX, currentGeoPoint.longitude);
        minY = MIN(minY, currentGeoPoint.latitude);
        maxX = MAX(maxX, currentGeoPoint.longitude);
        maxY = MAX(maxY, currentGeoPoint.latitude);

        DPoint2d cartPoint;
        cartPoint.x = currentGeoPoint.longitude;
        cartPoint.y = currentGeoPoint.latitude;
        cartesianShape.push_back(cartPoint);
        }
 
    ASSERT_TRUE(cartesianShape.size() == shape.size());
    ASSERT_TRUE(minX < maxX);
    ASSERT_TRUE(minY < maxY);

      
    // Danish coordinate systems cannot be validated since their mathematical domain is smaller (and not square) to their user domain
    auto projectionCode = toto->GetProjectionCode();

    if (projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys34  ||
        projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys3499 ||
        projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys3401)
        {
        double latitudeStep = (maxY - minY) / 10.1;
        double longitudeStep = (maxX - minX) / 10.1;
        // Note that the tolerance added serves to prevent user domain error that may result from floating point rounding errors
        // as some projection methods validate on the exact same value we use but using a different computational path
        // that injects different rounding errors. The tolerance is 0.001 second of arc (or a max of 3 centimeters)
        for (double latitude = minY; latitude < maxY ; latitude += latitudeStep)
            {
            for (double longitude = minX; longitude < maxX; longitude += longitudeStep)
                {
                GeoPoint latLongPoint;
                latLongPoint.longitude = longitude;
                latLongPoint.latitude = latitude;
                latLongPoint.elevation = 0.0;
                DPoint3d outPoint;
                DPoint2d testPoint;
                testPoint.x = longitude;
                testPoint.y = latitude;
           
                if (bsiDPoint2d_isPointInConvexPolygonFixed(&testPoint, &(cartesianShape[0]), (int)cartesianShape.size(), 0))
                    {
                    // Even though the error is not fatal we stop since all it does is clutter the output in case of multiple errors.
                    auto returnCode = toto->CartesianFromLatLong(outPoint, latLongPoint);
                    EXPECT_TRUE(REPROJECT_Success == returnCode || 1 == returnCode);
                    }
                }
            }
        }
    else
        {
        // Normally any projection method other than Danish ones should be square
        ASSERT_TRUE(cartesianShape.size() == 5);

        // This fact will allow us not to test each point against the shape.

   
        double latitudeStep = (maxY - minY) / 10.1;
        double longitudeStep = (maxX - minX) / 10.1;
        // Note that the tolerance added serves to prevent user domain error that may result from floating point rounding errors
        // as some projection methods validate on the exact same value we use but using a different computational path
        // that injects different rounding errors. The tolerance is 0.001 second of arc (or a max of 3 centimeters)
        for (double latitude = minY; latitude < maxY ; latitude += latitudeStep)
            {
            for (double longitude = minX; longitude < maxX; longitude += longitudeStep)
                {
                GeoPoint latLongPoint;
                latLongPoint.longitude = longitude;
                latLongPoint.latitude = latitude;
                latLongPoint.elevation = 0.0;
                DPoint3d outPoint;

                // Even though the error is not fatal we stop since all it does is clutter the output in case of multiple errors.         
                ASSERT_TRUE(REPROJECT_Success == toto->CartesianFromLatLong(outPoint, latLongPoint));
                }
            }
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

    // We do not check the OSGB GCS since usually those are not available because of the
    // abscense of the 
    const WChar* keyname = GetParam().c_str();
    WString theKeyname(keyname);
    if (theKeyname.CompareTo(L"OSGB-GPS-2002") == 0 || theKeyname.CompareTo(L"OSGB-GPS-1997") == 0)
        return;

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(keyname);
    
    // Check transformation properties
    EXPECT_TRUE(!toto.IsNull());
    
    EXPECT_TRUE(toto.IsValid());

    auto projectionCode = toto->GetProjectionCode();

    // The following set cannot be converted to any WKT flavor
    if (projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorAffinePostProcess ||
	    projectionCode == GeoCoordinates::BaseGCS::pcvLambertConformalConicAffinePostProcess ||
	    projectionCode == GeoCoordinates::BaseGCS::pcvAzimuthalEquidistantElevatedEllipsoid   ||
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
        WString WKT;
    
        EXPECT_TRUE(toto->GetWellKnownText(WKT, currentFlavor, true) == SUCCESS);
    
        EXPECT_TRUE(WKT.size() > 0);

        WString compoundWKT;
    
        EXPECT_TRUE(toto->GetCompoundCSWellKnownText(compoundWKT, currentFlavor, false) == SUCCESS);
    
        EXPECT_TRUE(compoundWKT.size() > 0);

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
TEST_P (GCSUnaryTester, WKTGenerateThenParseTest)
    {
    // We do not check the OSGB GCS since usually those are not available because of the
    // abscense of the 
    const WChar* keyname = GetParam().c_str();
    WString theKeyname(keyname);
    if (theKeyname.CompareTo(L"OSGB-GPS-2002") == 0 || theKeyname.CompareTo(L"OSGB-GPS-1997") == 0)
        return;

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(keyname);
    
    // Check transformation properties
    EXPECT_TRUE(!toto.IsNull());
    
    EXPECT_TRUE(toto.IsValid());

    auto projectionCode = toto->GetProjectionCode();

    // The following set cannot be converted to any WKT flavor
    if (projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorAffinePostProcess ||
	    projectionCode == GeoCoordinates::BaseGCS::pcvLambertConformalConicAffinePostProcess ||
	    projectionCode == GeoCoordinates::BaseGCS::pcvAzimuthalEquidistantElevatedEllipsoid   ||
	    projectionCode == GeoCoordinates::BaseGCS::pcvModifiedStereographic   ||
	    projectionCode == GeoCoordinates::BaseGCS::pcvBipolarObliqueConformalConic   ||
	    projectionCode == GeoCoordinates::BaseGCS::pcvHotineObliqueMercator1UV  ||
	    projectionCode == GeoCoordinates::BaseGCS::pcvHotineObliqueMercator2UV ||
        projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys3401 ||
        projectionCode == GeoCoordinates::BaseGCS::pcvTotalUniversalTransverseMercator ||  
        projectionCode == GeoCoordinates::BaseGCS::pcvTotalTransverseMercatorBF ||
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
        WString WKT;
    
        EXPECT_TRUE(toto->GetWellKnownText(WKT, currentFlavor, true) == SUCCESS);
    
        EXPECT_TRUE(WKT.size() > 0);

        // Now we recreate the GCS
        GeoCoordinates::BaseGCSPtr recreatedGCS = GeoCoordinates::BaseGCS::CreateGCS();
    
        EXPECT_TRUE(!recreatedGCS.IsNull());
    
        if (!recreatedGCS.IsNull())
            {
            if (SUCCESS != recreatedGCS->InitFromWellKnownText(NULL, NULL, currentFlavor, WKT.c_str()))
                EXPECT_TRUE(SUCCESS == recreatedGCS->InitFromWellKnownText(NULL, NULL, currentFlavor, WKT.c_str()));
            
            EXPECT_TRUE(recreatedGCS->IsEquivalent(*toto));
            }

        WString compoundWKT;
    
     //   toto->SetVerticalDatumCode(GeoCoordinates::VertDatumCode::vdcGeoid);

        EXPECT_TRUE(toto->GetCompoundCSWellKnownText(compoundWKT, currentFlavor, false) == SUCCESS);
    
        EXPECT_TRUE(compoundWKT.size() > 0);

        recreatedGCS = GeoCoordinates::BaseGCS::CreateGCS();
    
        
        EXPECT_TRUE(!recreatedGCS.IsNull());
    
        if (!recreatedGCS.IsNull())
            {
            EXPECT_TRUE(SUCCESS == recreatedGCS->InitFromWellKnownText(NULL, NULL, currentFlavor, compoundWKT.c_str()));
                
            EXPECT_TRUE(recreatedGCS->IsEquivalent(*toto));
            }

        }
    }
    
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
