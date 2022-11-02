//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <Bentley/Desktop/FileSystem.h>
#include <GeoCoord/BaseGeoCoord.h>
#include <GeoCoord/BaseGeoTiffKeysList.h>

#include "GeoCoordTestCommon.h"

using namespace ::testing;

using namespace GeoCoordinates;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class LocalTransformerUnitTests : public ::testing::Test
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        LocalTransformerUnitTests() {};
        ~LocalTransformerUnitTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalTransformerUnitTests, BasicHelmertTransformTest)
{
    double parameters[12] = {1.0, 0.0, 1.2, 1.3, 1.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    LocalTransformerPtr localTransform = LocalTransformer::CreateLocalTransformer(TRANSFORM_Helmert, parameters);

    ASSERT_TRUE(localTransform.IsValid());

    HelmertLocalTransformer* helmertTransform = dynamic_cast<HelmertLocalTransformer*>(localTransform.get());

    ASSERT_TRUE(helmertTransform != nullptr);

    EXPECT_NEAR(helmertTransform->GetA(), 1.0, 0.000001);
    EXPECT_NEAR(helmertTransform->GetB(), 0.0, 0.000001);
    EXPECT_NEAR(helmertTransform->GetC(), 1.2, 0.000001);
    EXPECT_NEAR(helmertTransform->GetD(), 1.3, 0.000001);
    EXPECT_NEAR(helmertTransform->GetE(), 1.4, 0.000001);

    helmertTransform->SetA(0.0); // This old code would have triggeres a division by zero
    helmertTransform->SetB(1.0); 
    helmertTransform->SetC(2.0); 
    helmertTransform->SetD(2.1); 
    helmertTransform->SetE(2.2); 

    EXPECT_NEAR(helmertTransform->GetA(), 0.0, 0.000001);
    EXPECT_NEAR(helmertTransform->GetB(), 1.0, 0.000001);
    EXPECT_NEAR(helmertTransform->GetC(), 2.0, 0.000001);
    EXPECT_NEAR(helmertTransform->GetD(), 2.1, 0.000001);
    EXPECT_NEAR(helmertTransform->GetE(), 2.2, 0.000001);

    parameters[0] = 12.1;
    parameters[1] = 24.3;
    parameters[2] = 12.3;    
    parameters[3] = 32.3;    
    parameters[4] = 44.3;

    helmertTransform->ReadParameters(parameters);
    EXPECT_NEAR(helmertTransform->GetA(), 12.1, 0.000001);
    EXPECT_NEAR(helmertTransform->GetB(), 24.3, 0.000001);
    EXPECT_NEAR(helmertTransform->GetC(), 12.3, 0.000001);
    EXPECT_NEAR(helmertTransform->GetD(), 32.3, 0.000001);
    EXPECT_NEAR(helmertTransform->GetE(), 44.3, 0.000001);

    parameters[0] = 0.3;
    parameters[1] = 0.3;
    parameters[2] = 0.3;    
    parameters[3] = 0.3;    
    parameters[4] = 0.3;

    uint16_t transformType;
    helmertTransform->SaveParameters(transformType, parameters);
    EXPECT_TRUE(transformType == (uint16_t)TRANSFORM_Helmert);
    EXPECT_NEAR(helmertTransform->GetA(), parameters[0], 0.000001);
    EXPECT_NEAR(helmertTransform->GetB(), parameters[1], 0.000001);
    EXPECT_NEAR(helmertTransform->GetC(), parameters[2], 0.000001);
    EXPECT_NEAR(helmertTransform->GetD(), parameters[3], 0.000001);
    EXPECT_NEAR(helmertTransform->GetE(), parameters[4], 0.000001);

    LocalTransformerPtr secondTransform = helmertTransform->Copy();

    HelmertLocalTransformer* helmertTransform2 = dynamic_cast<HelmertLocalTransformer*>(secondTransform.get());

    ASSERT_TRUE(helmertTransform2 != nullptr);

    ASSERT_TRUE(secondTransform.IsValid());

    EXPECT_TRUE(secondTransform->IsEquivalent(helmertTransform));
    EXPECT_NEAR(helmertTransform2->GetA(), parameters[0], 0.000001);
    EXPECT_NEAR(helmertTransform2->GetB(), parameters[1], 0.000001);
    EXPECT_NEAR(helmertTransform2->GetC(), parameters[2], 0.000001);
    EXPECT_NEAR(helmertTransform2->GetD(), parameters[3], 0.000001);
    EXPECT_NEAR(helmertTransform2->GetE(), parameters[4], 0.000001);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalTransformerUnitTests, GeometricHelmertTransformTest)
{
    // Defines a neutral transform
    double parameters[12] = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    LocalTransformerPtr localTransform = LocalTransformer::CreateLocalTransformer(TRANSFORM_Helmert, parameters);

    ASSERT_TRUE(localTransform.IsValid());

    HelmertLocalTransformer* helmertTransform = dynamic_cast<HelmertLocalTransformer*>(localTransform.get());

    ASSERT_TRUE(helmertTransform != nullptr);

    DPoint3d inCartesian;
    inCartesian.x = 0.0;
    inCartesian.y = 0.0;
    inCartesian.z = 0.0;

    DPoint3d outInternalCartesian;
    localTransform->InternalCartesianFromCartesian (outInternalCartesian, inCartesian);

    EXPECT_EQ(outInternalCartesian.x, 0.0);
    EXPECT_EQ(outInternalCartesian.y, 0.0);
    EXPECT_EQ(outInternalCartesian.z, 0.0);

    inCartesian.x = 100.0;
    inCartesian.y = 100.0;
    inCartesian.z = 100.0;

    localTransform->InternalCartesianFromCartesian (outInternalCartesian, inCartesian);

    EXPECT_EQ(outInternalCartesian.x, 100.0);
    EXPECT_EQ(outInternalCartesian.y, 100.0);
    EXPECT_EQ(outInternalCartesian.z, 100.0);

    DPoint3d inInternalCartesian;
    inInternalCartesian.x = 100.0;
    inInternalCartesian.y = 100.0;
    inInternalCartesian.z = 100.0;

    DPoint3d outCartesian;
    localTransform->CartesianFromInternalCartesian(outCartesian, inInternalCartesian);

    EXPECT_EQ(outCartesian.x, 100.0);
    EXPECT_EQ(outCartesian.y, 100.0);
    EXPECT_EQ(outCartesian.z, 100.0);

    // =================================================================

    parameters[0] = 0.0;
    parameters[1] = 1.0;
    parameters[2] = -1000.0;
    parameters[3] = -2000.0;
    parameters[4] = -3000.0;

    // Change local transform
    localTransform = LocalTransformer::CreateLocalTransformer(TRANSFORM_Helmert, parameters);

    helmertTransform = dynamic_cast<HelmertLocalTransformer*>(localTransform.get());

    ASSERT_TRUE(helmertTransform != nullptr);

    inCartesian.x = 10.0;
    inCartesian.y = 0.0;
    inCartesian.z = 0.0;

    localTransform->InternalCartesianFromCartesian (outInternalCartesian, inCartesian);

    EXPECT_EQ(outInternalCartesian.x, -1000.0);
    EXPECT_EQ(outInternalCartesian.y, -1990.0);
    EXPECT_EQ(outInternalCartesian.z, -3000.0);

    inCartesian.x = 0.0;
    inCartesian.y = 100.0;
    inCartesian.z = 100.0;

    localTransform->InternalCartesianFromCartesian (outInternalCartesian, inCartesian);

    EXPECT_EQ(outInternalCartesian.x, -1100.0);
    EXPECT_EQ(outInternalCartesian.y, -2000.0);
    EXPECT_EQ(outInternalCartesian.z, -2900.0);

    inInternalCartesian.x = 0.0;
    inInternalCartesian.y = 100.0;
    inInternalCartesian.z = 100.0;

    localTransform->CartesianFromInternalCartesian(outCartesian, inInternalCartesian);

    EXPECT_EQ(outCartesian.x, 2100.0);
    EXPECT_EQ(outCartesian.y, -1000.0);
    EXPECT_EQ(outCartesian.z, 3100.0);

    localTransform->InternalCartesianFromCartesian(outInternalCartesian, outCartesian);

    EXPECT_NEAR(outInternalCartesian.x, inInternalCartesian.x, 0.00001);
    EXPECT_NEAR(outInternalCartesian.y, inInternalCartesian.y, 0.00001);
    EXPECT_NEAR(outInternalCartesian.z, inInternalCartesian.z, 0.00001);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalTransformerUnitTests, GeometricHelmertTransformTest2)
{
    // Defines a neutral transform (These parameters come from Ontario Line GCS 2020 (City of Toronto)
    double parameters[12] = { 0.999993442599901000, 0.000000445056040829, 20.038650337876800000, 253.670223126782000000, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

    LocalTransformerPtr localTransform = LocalTransformer::CreateLocalTransformer(TRANSFORM_Helmert, parameters);

    ASSERT_TRUE(localTransform.IsValid());

    HelmertLocalTransformer* helmertTransform = dynamic_cast<HelmertLocalTransformer*>(localTransform.get());

    ASSERT_TRUE(helmertTransform != nullptr);

    DPoint3d inCartesian;
    inCartesian.x = 310791.742000000000000000;
    inCartesian.y = 4832448.663000000000000000;
    inCartesian.z = 0.0;

    DPoint3d outInternalCartesian;
    localTransform->InternalCartesianFromCartesian(outInternalCartesian, inCartesian);

    EXPECT_NEAR(outInternalCartesian.x, 310807.591954069000000000, 0.001);
    EXPECT_NEAR(outInternalCartesian.y, 4832670.783243530000000000, 0.001);
    EXPECT_EQ(outInternalCartesian.z, 0.0);

    DPoint3d inInternalCartesian;
    inInternalCartesian.x = outInternalCartesian.x;
    inInternalCartesian.y = outInternalCartesian.y;
    inInternalCartesian.z = 0.0;

    DPoint3d outCartesian;
    localTransform->CartesianFromInternalCartesian(outCartesian, inInternalCartesian);

    EXPECT_NEAR(outCartesian.x, inCartesian.x, 0.001);
    EXPECT_NEAR(outCartesian.y, inCartesian.y, 0.001);
    EXPECT_NEAR(outCartesian.z, 0.0, 0.001);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalTransformerUnitTests, JSONHelmertTransformTest)
{
    // Defines a neutral transform
    double parameters[12] = {1.0, 0.0, 10.0, 20.0, 30.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    LocalTransformerPtr localTransform = LocalTransformer::CreateLocalTransformer(TRANSFORM_Helmert, parameters);

    ASSERT_TRUE(localTransform.IsValid());

    Json::Value outValue;
    EXPECT_TRUE(SUCCESS == localTransform->ToJson(outValue, false));

    ASSERT_TRUE(!outValue["rotDeg"].isNull());
    EXPECT_NEAR(0.0, outValue["rotDeg"].asDouble(), 0.00001);
    ASSERT_TRUE(!outValue["scale"].isNull());
    EXPECT_NEAR(1.0, outValue["scale"].asDouble(), 0.00001);
    ASSERT_TRUE(!outValue["translationX"].isNull());
    EXPECT_NEAR(10.0, outValue["translationX"].asDouble(), 0.00001);
    ASSERT_TRUE(!outValue["translationY"].isNull());
    EXPECT_NEAR(20.0, outValue["translationY"].asDouble(), 0.00001);
    ASSERT_TRUE(!outValue["translationZ"].isNull());
    EXPECT_NEAR(30.0, outValue["translationZ"].asDouble(), 0.00001);

    Utf8String errorMessage;
    LocalTransformerPtr localTransform2 = HelmertLocalTransformer::CreateFromJson(outValue, errorMessage);

    Utf8String resultString2 = outValue.toStyledString();

    ASSERT_TRUE(localTransform2.IsValid());

    EXPECT_TRUE(LocalTransformer::IsEquivalent(localTransform, localTransform2));

    Json::Value outValue2;
    EXPECT_TRUE(SUCCESS == localTransform->ToJson(outValue2, true));

    ASSERT_TRUE(!outValue2["helmert2DWithZOffset"].isNull());

    Json::Value helmert = outValue2["helmert2DWithZOffset"];

    ASSERT_TRUE(!helmert["rotDeg"].isNull());
    EXPECT_NEAR(0.0, helmert["rotDeg"].asDouble(), 0.00001);
    ASSERT_TRUE(!helmert["scale"].isNull());
    EXPECT_NEAR(1.0, helmert["scale"].asDouble(), 0.00001);
    ASSERT_TRUE(!helmert["translationX"].isNull());
    EXPECT_NEAR(10.0, helmert["translationX"].asDouble(), 0.00001);
    ASSERT_TRUE(!helmert["translationY"].isNull());
    EXPECT_NEAR(20.0, helmert["translationY"].asDouble(), 0.00001);
    ASSERT_TRUE(!helmert["translationZ"].isNull());
    EXPECT_NEAR(30.0, helmert["translationZ"].asDouble(), 0.00001);

    LocalTransformerPtr localTransform3 = LocalTransformer::CreateLocalTransformerFromJson(outValue2, errorMessage);

    ASSERT_TRUE(localTransform3.IsValid());

    EXPECT_TRUE(LocalTransformer::IsEquivalent(localTransform, localTransform3));
}

#if (0) //Something wrong with JSON parsing ... needs to investigate.
/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained Datum that uses grid shift files and custom ellipsoid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalTransformerUnitTests, InvalidJson_Test)
{
    Utf8String errorMessage;

    Utf8String localTransform1 = "   { \"helmert2DWithZOffset\" : {"
        "        \"rotDeg\": 12.4,"
        "        \"scale\": 0.0,"
        "        \"translationX\": 1.22,"
        "        \"translationY\": 2.12,"
        "        \"translationZ\": 0.01"
        "      } }";

    Json::Value resultJson;
    EXPECT_TRUE(Json::Reader::Parse(localTransform1, resultJson));

    LocalTransformerPtr localTransformObj1 = LocalTransformer::CreateLocalTransformerFromJson(resultJson, errorMessage);

    EXPECT_TRUE(!localTransformObj1.IsValid()); // Scale cannot be zero


    // -------------------------------------
    Utf8String localTransform2 = "{ \"helmert2DWithZOffset\" : {"
        "        \"rotDeg\": 12.4,"
        "        \"scale\": 0.0,"
        "        \"translationZ\": 0.01"
        "      } }";

    Json::Value resultJson2;
    EXPECT_TRUE(Json::Reader::Parse(localTransform2, resultJson2));

    LocalTransformerPtr localTransformObj2 = LocalTransformer::CreateLocalTransformerFromJson(resultJson2, errorMessage);

    EXPECT_TRUE(!localTransformObj2.IsValid()); // All parameters must be provided

    // -------------------------------------
    Utf8String localTransform3 = "   { \"helmert2DWithZOffset\" : {"
        "        \"rotDeg\": 12.4,"
        "        \"scale\": 1.0,"
        "        \"translationY\": 2.12,"
        "        \"translationZ\": 0.01"
        "      } }";

    Json::Value resultJson3;
    EXPECT_TRUE(Json::Reader::Parse(localTransform3, resultJson3));

    LocalTransformerPtr localTransformObj3 = LocalTransformer::CreateLocalTransformerFromJson(resultJson3, errorMessage);

    EXPECT_TRUE(!localTransformObj3.IsValid()); // All parameters must be provided

    // -------------------------------------
    Utf8String localTransform4 = "   { \"helmert2DWithZOffset\" : {"
        "        \"rotDeg\": 12.4,"
        "        \"scale\": 1.0,"
        "        \"translationX\": 2.12,"
        "        \"translationZ\": 0.01"
        "      } }";

    Json::Value resultJson4;
    EXPECT_TRUE(Json::Reader::Parse(localTransform4, resultJson4));

    LocalTransformerPtr localTransformObj4 = LocalTransformer::CreateLocalTransformerFromJson(resultJson4, errorMessage);

    EXPECT_TRUE(!localTransformObj4.IsValid()); // All parameters must be provided

    // -------------------------------------
    Utf8String localTransform5 = "   { \"helmert2DWithZOffset\" : {"
        "        \"rotDeg\": 12.4,"
        "        \"scale\": 1.0,"
        "        \"translationX\": 2.12,"
        "        \"translationY\": 0.01"
        "      } }";

    Json::Value resultJson5;
    EXPECT_TRUE(Json::Reader::Parse(localTransform5, resultJson5));

    LocalTransformerPtr localTransformObj5 = LocalTransformer::CreateLocalTransformerFromJson(resultJson5, errorMessage);

    EXPECT_TRUE(!localTransformObj5.IsValid()); // All parameters must be provided

    // -------------------------------------
    Utf8String localTransform6 = "   { \"helmert2DWithZOffset\" : {"
        "        \"scale\": 1.0,"
        "        \"translationX\": 2.12,"
        "        \"translationY\": 2.12,"
        "        \"translationY\": 0.01"
        "      } }";

    Json::Value resultJson6;
    EXPECT_TRUE(Json::Reader::Parse(localTransform6, resultJson6));

    LocalTransformerPtr localTransformObj6 = LocalTransformer::CreateLocalTransformerFromJson(resultJson6, errorMessage);

    EXPECT_TRUE(!localTransformObj6.IsValid()); // All parameters must be provided


    // -------------------------------------
    Utf8String localTransform7 = "   { \"helmert2DWithZOffset\" : {"
        "        \"rotDeg\": 12.4,"
        "        \"translationX\": 2.12,"
        "        \"translationY\": 2.12,"
        "        \"translationY\": 0.01"
        "      } }";

    Json::Value resultJson7;
    EXPECT_TRUE(Json::Reader::Parse(localTransform7, resultJson7));

    LocalTransformerPtr localTransformObj7 = LocalTransformer::CreateLocalTransformerFromJson(resultJson7, errorMessage);

    EXPECT_TRUE(!localTransformObj7.IsValid()); // All parameters must be provided
}
#endif