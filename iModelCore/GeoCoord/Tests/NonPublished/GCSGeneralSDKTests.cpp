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
class GCSGeneralSDKTests : public ::testing::TestWithParam< Utf8String >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        GCSGeneralSDKTests() {};
        ~GCSGeneralSDKTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* test all transformation to full json then back
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSGeneralSDKTests, GCSTransformToFullJsonThenBack)
    {
    const bvector<Utf8String>& listOfGCS = GeoCoordTestCommon::GetRepresentativeListOfGCS();

    double parameters[12] = {1.0, 0.0, 10.0, 20.0, 30.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    GeoCoordinates::LocalTransformerPtr localTransform1 = GeoCoordinates::LocalTransformer::CreateLocalTransformer(GeoCoordinates::TRANSFORM_Helmert, parameters);

    double parameters2[12] = {0.0, 1.0, 0.0, -20.0, 300.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    GeoCoordinates::LocalTransformerPtr localTransform2 = GeoCoordinates::LocalTransformer::CreateLocalTransformer(GeoCoordinates::TRANSFORM_Helmert, parameters2);

    double parameters3[12] = {2, 2, 0.0, -200.0, 450.0, 34.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    GeoCoordinates::LocalTransformerPtr localTransform3 = GeoCoordinates::LocalTransformer::CreateLocalTransformer(GeoCoordinates::TRANSFORM_Helmert, parameters3);

    for (int index = 0 ; index < listOfGCS.size() ; index++)
        {
        std::cerr << "Testing " << index << " of " << listOfGCS.size() << std::endl;

        Utf8String theKeyname(listOfGCS[index]);
        GeoCoordinates::BaseGCSPtr currentGCS = GeoCoordinates::BaseGCS::CreateGCS(theKeyname.c_str());

        if (!currentGCS.IsValid() || ! currentGCS->IsValid())
            continue;  // This occurs for weird GCS like OSGB-GPS-1997

        // vertical datum set
        int remainder;
        GeoCoordinates::DatumCP theDatum = currentGCS->GetDatum(); // May be null for ellipsoid based GCS
        
        if (nullptr != theDatum && (theDatum->IsNAD27() || theDatum->IsNAD83()))
            {
            remainder = index % 5;
            if (remainder == 0)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);
            else if (remainder == 1)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);
            else if (remainder == 2)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcNGVD29);
            else if (remainder == 3)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcNAVD88);
            else if (remainder == 4)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcLocalEllipsoid);
            }
        else
            {
            remainder = index % 3;

            if (remainder == 0)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcGeoid);
            else if (remainder == 1)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);
            else if (remainder == 2)
                currentGCS->SetVerticalDatumCode(GeoCoordinates::vdcLocalEllipsoid);
            }

        remainder = (index + 1) % 5;

        if (remainder == 0)
            currentGCS->SetLocalTransformer(localTransform1.get());
        else if (remainder == 1)
            currentGCS->SetLocalTransformer(localTransform2.get());
        else if (remainder == 2)
            currentGCS->SetLocalTransformer(localTransform3.get());

        if (currentGCS.IsValid() && currentGCS->IsValid())
            {
            Json::Value result ;

            if (SUCCESS == currentGCS->ToJson(result, true))
                {
                // Transform to string (for debug purposes)
                Utf8String resultString = result.toStyledString();

                // Make sure that domain is specified
                EXPECT_TRUE(!result["horizontalCRS"].isNull());
                EXPECT_TRUE(!result["verticalCRS"].isNull());

                Utf8String resultString2 = result.toStyledString();

                // Sabotage GCS name to make sure everything is parsed
                result["horizontalCRS"]["id"] = "XYZ";
            
                GeoCoordinates::BaseGCSPtr resultGCS = GeoCoordinates::BaseGCS::CreateGCS();

                Utf8String errMessage;
                EXPECT_EQ(SUCCESS, resultGCS->FromJson(result, errMessage)) << errMessage.c_str();

                EXPECT_TRUE(currentGCS->IsEquivalent(*resultGCS));
                
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMinimumLatitude(), resultGCS->GetMinimumLatitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMinimumLongitude(), resultGCS->GetMinimumLongitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMaximumLatitude(), resultGCS->GetMaximumLatitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMaximumLongitude(), resultGCS->GetMaximumLongitude()));

                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMinimumUsefulLatitude(), resultGCS->GetMinimumUsefulLatitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMinimumUsefulLongitude(), resultGCS->GetMinimumUsefulLongitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMaximumUsefulLatitude(), resultGCS->GetMaximumUsefulLatitude()));
                EXPECT_TRUE(GeoCoordTestCommon::doubleSame(currentGCS->GetMaximumUsefulLongitude(), resultGCS->GetMaximumUsefulLongitude()));

                EXPECT_STREQ(currentGCS->GetDescription(), resultGCS->GetDescription());
                Utf8String source1, source2;
                EXPECT_STREQ(currentGCS->GetSource(source1), resultGCS->GetSource(source2));
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* test all transformation to horizontal json then back
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSGeneralSDKTests, GCSTransformtoHorizontalJsonThenBack)
    {
    const bvector<Utf8String>& listOfGCS = GeoCoordTestCommon::GetRepresentativeListOfGCS();

    for (int index = 0 ; index < listOfGCS.size() ; index++)
        {
        std::cerr << "Testing " << index << " of " << listOfGCS.size() << std::endl;

        Utf8String theKeyname(listOfGCS[index]);
        GeoCoordinates::BaseGCSPtr currentGCS = GeoCoordinates::BaseGCS::CreateGCS(theKeyname.c_str());

        if (currentGCS.IsValid() && currentGCS->IsValid())
            {
            Json::Value result ;

            if (SUCCESS == currentGCS->ToHorizontalJson(result, true))
                {
                // Transform to string (for debug purposes)
                Utf8String resultString = result.toStyledString();

                // Make sure that domain is specified
                EXPECT_TRUE(!result["extent"].isNull());
                EXPECT_TRUE(!result["extent"]["southWest"].isNull());
                EXPECT_TRUE(!result["extent"]["northEast"].isNull());

                // Get domain bounds
                double minLat = result["extent"]["southWest"]["latitude"].asDouble();
                double minLong = result["extent"]["southWest"]["longitude"].asDouble();
                double maxLat = result["extent"]["northEast"]["latitude"].asDouble();
                double maxLong = result["extent"]["northEast"]["longitude"].asDouble();

                EXPECT_TRUE(minLat < maxLat);

                // TODO May not be true in JSON format.
                EXPECT_TRUE(minLong < maxLong);

                // Sabotage GCS name to make sure everything is parsed
                result["id"] = "XYZ";
            
                GeoCoordinates::BaseGCSPtr resultGCS = GeoCoordinates::BaseGCS::CreateGCS();

                Utf8String errMessage;
                EXPECT_EQ(SUCCESS, resultGCS->FromHorizontalJson(result, errMessage)) << errMessage.c_str();

                EXPECT_TRUE(currentGCS->IsEquivalent(*resultGCS));

                Utf8String resultString2 = result.toStyledString();
                }
            }
        }
    }




