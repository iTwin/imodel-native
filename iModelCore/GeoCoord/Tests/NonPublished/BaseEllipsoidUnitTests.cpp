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
class BaseEllipsoidUnitTests : public ::testing::Test
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        BaseEllipsoidUnitTests() {};
        ~BaseEllipsoidUnitTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* Test for IsDeprecated
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseEllipsoidUnitTests, IsDeprecatedTest)
{
    // Datum deprecation test
    GeoCoordinates::EllipsoidCP deprecatedEllipsoid = GeoCoordinates::Ellipsoid::CreateEllipsoid("BESSEL-CH");

    ASSERT_TRUE(deprecatedEllipsoid != nullptr && deprecatedEllipsoid->IsValid());

    ASSERT_TRUE(deprecatedEllipsoid->IsDeprecated());

    GeoCoordinates::EllipsoidCP nonDeprecatedEllipsoid = GeoCoordinates::Ellipsoid::CreateEllipsoid("Borneo");

    ASSERT_TRUE(nonDeprecatedEllipsoid != nullptr && nonDeprecatedEllipsoid->IsValid());

    ASSERT_TRUE(!nonDeprecatedEllipsoid->IsDeprecated());

    deprecatedEllipsoid->Destroy();
    nonDeprecatedEllipsoid->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Basic Ellipsoid JSON test
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseEllipsoidUnitTests, BasicJSONTests1ShortForm)
{
    GeoCoordinates::EllipsoidCP wgs84Ellipsoid = GeoCoordinates::Ellipsoid::CreateEllipsoid("WGS84");

    Json::Value result;
    BeJsValue resultLong(result);
    ASSERT_TRUE(SUCCESS == wgs84Ellipsoid->ToJson(resultLong));

    EXPECT_TRUE(!resultLong["description"].isNull());
    EXPECT_TRUE(!resultLong["id"].isNull());
    EXPECT_TRUE(!resultLong["epsg"].isNull());
    EXPECT_TRUE(!resultLong["source"].isNull());
    EXPECT_TRUE(!resultLong["polarRadius"].isNull());
    EXPECT_TRUE(!resultLong["equatorialRadius"].isNull());

    Json::Value resultShort;
    ASSERT_TRUE(SUCCESS == wgs84Ellipsoid->ToJson(resultShort, true));

    EXPECT_TRUE(resultShort["description"].isNull());
    EXPECT_TRUE(resultShort["source"].isNull());
    EXPECT_TRUE(resultShort["epsg"].isNull());

    EXPECT_TRUE(!resultShort["id"].isNull());
    EXPECT_TRUE(!resultShort["polarRadius"].isNull());
    EXPECT_TRUE(!resultShort["equatorialRadius"].isNull());

    EXPECT_TRUE(resultShort["id"].asString() == resultLong["id"].asString());
    EXPECT_TRUE(resultShort["polarRadius"].asDouble() == resultLong["polarRadius"].asDouble());
    EXPECT_TRUE(resultShort["equatorialRadius"].asDouble() == resultLong["equatorialRadius"].asDouble());

    wgs84Ellipsoid->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* Test using a JSON defined self contained ellipsoid
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BaseEllipsoidUnitTests, CreateAFullySelfContainedEllipsoid_Test1)
{
    // custom ellipsoid with unique radius.
    Utf8String customEllipsoid1  =  "    {"
        "        \"id\": \"CustomEllipsoid\","
        "        \"description\": \"Custom ellipsoid description\","
        "        \"source\": \"Custom Ellipsoid source\","
        "        \"equatorialRadius\": 6378120.0,"
        "        \"polarRadius\": 6356794.719195305951"
        "    }";

    GeoCoordinates::EllipsoidP theEllipsoid = const_cast<GeoCoordinates::EllipsoidP>(GeoCoordinates::Ellipsoid::CreateEllipsoid());

    Utf8String errorMessage;
    ASSERT_TRUE(SUCCESS == theEllipsoid->FromJson(Json::Value::From(customEllipsoid1), errorMessage));
    Utf8String source;
    EXPECT_TRUE(Utf8String(theEllipsoid->GetName()) == "CustomEllipsoid");
    EXPECT_TRUE(Utf8String(theEllipsoid->GetDescription()) == "Custom ellipsoid description");
    EXPECT_TRUE(Utf8String(theEllipsoid->GetSource(source)) == "Custom Ellipsoid source");
    EXPECT_TRUE(theEllipsoid->GetEquatorialRadius() == 6378120.0);
    EXPECT_TRUE(theEllipsoid->GetPolarRadius() == 6356794.719195305951);
    theEllipsoid->Destroy();
}
