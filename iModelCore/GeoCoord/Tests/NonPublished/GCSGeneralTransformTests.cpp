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
static bool s_AngleInDegreesEqual (double angle1, double angle2, double tolerance)
    {
    // Make sure tolerance is positive
    if (tolerance < 0.0)
        tolerance = -tolerance;

    if (angle1 - tolerance < angle2 && angle1 + tolerance >= angle2)
        return true;
    
    // Even if the values are not equal they may still be equal angular-wise
    // Normalise the values
    while (angle1 < -180.0)
        angle1 += 360.0;

    while (angle1 > 180.0)
        angle1 -= 360.0;

    while (angle2 < -180.0)
        angle2 += 360.0;

    while (angle2 > 180.0)
        angle2 -= 360.0;


    if (angle1 - tolerance < angle2 && angle1 + tolerance >= angle2)
        return true;

    return false;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class GCSGeneralTransformTests : public ::testing::TestWithParam< Utf8String >
    {   
      public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(false); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        GCSGeneralTransformTests() {};
        ~GCSGeneralTransformTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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
        areaFactor = PolygonOps::Area (pPointArray, numPoint);

    for (i0 = numPoint - 1, i1 = 0; i1 < numPoint; i0 = i1++)
        {
        dot = pPoint->CrossProductToPoints (pPointArray[i0], pPointArray[i1]);
        if (dot * areaFactor < 0.0)
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* UserDomainCartesianConversionTest
* This test makes sure that no error is reported when converting from a lat/long
* pair reported in the user domain to a cartesian coordinate.
* The only exception to this rule is the Danish coordinate system
* that have highly complex mathematical domain that are fully inscribed in the
* use domain extent still not convering the corners.
* Notice that conversion from lat/long in the GCS datum to cartesian only is performed
* so we do not test as a side line the conversion of the datums.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P (GCSGeneralTransformTests, UserDomainConvertionTest)
    {
    // We do not check the OSGB GCS since usually those are not available because of the
    // abscense of the 
    const Utf8CP keyname = GetParam().c_str();
    Utf8String theKeyname(keyname);
    if (theKeyname.CompareTo("OSGB-GPS-2002") == 0 || 
        theKeyname.CompareTo("OSGB-GPS-2015") == 0 ||
        theKeyname.CompareTo("OSGB-GPS-1997") == 0)
        return;

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(keyname);

    ASSERT_TRUE(toto.IsValid() && toto->IsValid());

    double minUsefulLongitude = toto->GetMinimumUsefulLongitude();
    double maxUsefulLongitude = toto->GetMaximumUsefulLongitude();
    double minUsefulLatitude = toto->GetMinimumUsefulLatitude();
    double maxUsefulLatitude = toto->GetMaximumUsefulLatitude();
    
    double minLongitude = toto->GetMinimumLongitude();
    double maxLongitude = toto->GetMaximumLongitude();
    double minLatitude = toto->GetMinimumLatitude();
    double maxLatitude = toto->GetMaximumLatitude();

    // Danish coordinate systems cannot be validated since their mathematical domain is smaller (and not square) to their user domain
    auto projectionCode = toto->GetProjectionCode();

    if (projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys34  ||
        projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys3499 ||
        projectionCode == GeoCoordinates::BaseGCS::pcvTransverseMercatorDenmarkSys3401)
        return;
    
    if (projectionCode != GeoCoordinates::BaseGCS::pcvUnity)
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

/*---------------------------------------------------------------------------------**//**
* MathematicalDomainCartesianConversionTest
* This test makes sure that no error is reported when converting from a lat/long
* pair reported in the mathematical domain to a cartesian coordinate.
* Notice that conversion from lat/long in the GCS datum to cartesian only is performed
* so we do not test as a side line the conversion of the datums.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P (GCSGeneralTransformTests, MathematicalDomainConvertionTest)
    {
    // We do not check the OSGB GCS since usually those are not available because of the
    // abscense of the 
    const Utf8CP keyname = GetParam().c_str();
    Utf8String theKeyname(keyname);
    if ((theKeyname.CompareTo("OSGB-GPS-1997") == 0) ||
        (theKeyname.CompareTo("OSGB-GPS-2002") == 0) ||
        (theKeyname.CompareTo("OSGB-GPS-2015") == 0))
        return;

    GeoCoordinates::BaseGCSPtr toto = GeoCoordinates::BaseGCS::CreateGCS(keyname);

    ASSERT_TRUE(toto.IsValid() && toto->IsValid());

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
    
INSTANTIATE_TEST_SUITE_P(GCSGeneralTransformTests_Combined,
                        GCSGeneralTransformTests,
                        ValuesIn(GeoCoordTestCommon::GetListOfGCS()));

