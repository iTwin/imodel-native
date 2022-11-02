//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <GeoCoord/Asset.h>

using namespace ::testing;
using namespace GeoCoordinates;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
class AssetUnitTests : public ::testing::Test {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AssetUnitTests, DeserializationNoPackage)
    {
    const char data[] = R"({
      "id": "Bentley.Australia",
      "somethingNew" : "somethingBlue"
      })";

    auto asset = Asset::Create(data);
    ASSERT_FALSE(nullptr == asset.get());
    ASSERT_STREQ("Bentley.Australia", asset->GetId().c_str());
    auto packages = asset->GetPackages();
    ASSERT_EQ(0, packages.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AssetUnitTests, DeserializationBadData)
    {
    const char missingId[] = R"({
      "description": "Australia",
      "userId": "331E3B90-6623-48F8-B372-E8DFAB71FF81",
      "ultimateRefId": "72adad30-c07c-465d-a1fe-2f2dfac950a4"
      })";

    ASSERT_TRUE(nullptr == Asset::Create("junk").get());
    ASSERT_TRUE(nullptr == Asset::Create(missingId).get());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AssetUnitTests, PersistenceRoundTrip)
    {
    const char jsonData[] = R"({
      "id": "Bentley.Australia",
      "description": "Australia",
      "userId": "331E3B90-6623-48F8-B372-E8DFAB71FF81",
      "ultimateRefId": "72adad30-c07c-465d-a1fe-2f2dfac950a4",
      "packages": [
        {
          "version": "1.0",
          "description": "Version 1",
          "date": "2020-12-16T21:36:21Z",
          "userId": "331E3B90-6623-48F8-B372-E8DFAB71FF81"
        },
        {
          "version": "2.0",
          "description": "Version 2",
          "date": "2021-12-16T21:36:21Z",
          "userId": "331E3B90-6623-48F8-B372-E8DFAB71FF81"
        }
      ]
      })";

    Json::Value valueIn(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonData, valueIn));
        
    AssetPtr expectedAsset = std::make_shared<Asset>();
    ASSERT_EQ(SUCCESS, expectedAsset->FromJson(valueIn));
    ASSERT_STREQ("Bentley.Australia", expectedAsset->GetId().c_str());
    ASSERT_STREQ("Australia", expectedAsset->GetDescription().c_str());
    ASSERT_EQ(2, expectedAsset->GetPackages().size());
    ASSERT_STREQ("1.0", expectedAsset->GetPackages()[0].GetVersion().c_str());
    ASSERT_STREQ("Version 1", expectedAsset->GetPackages()[0].GetDescription().c_str());
    ASSERT_STREQ("2.0", expectedAsset->GetPackages()[1].GetVersion().c_str());
    ASSERT_STREQ("Version 2", expectedAsset->GetPackages()[1].GetDescription().c_str());

    Json::Value valueOut(Json::objectValue);
    expectedAsset->ToJson(valueOut);

    AssetPtr actualAsset = std::make_shared<Asset>();
    ASSERT_EQ(SUCCESS, actualAsset->FromJson(valueOut));

    ASSERT_STREQ(expectedAsset->GetId().c_str(), actualAsset->GetId().c_str());
    ASSERT_STREQ(expectedAsset->GetDescription().c_str(), actualAsset->GetDescription().c_str());
    ASSERT_EQ(expectedAsset->GetPackages().size(), actualAsset->GetPackages().size());
    ASSERT_STREQ(expectedAsset->GetPackages()[0].GetVersion().c_str(), actualAsset->GetPackages()[0].GetVersion().c_str());
    ASSERT_STREQ(expectedAsset->GetPackages()[0].GetDescription().c_str(), actualAsset->GetPackages()[0].GetDescription().c_str());
    ASSERT_STREQ(expectedAsset->GetPackages()[1].GetVersion().c_str(), actualAsset->GetPackages()[1].GetVersion().c_str());
    ASSERT_STREQ(expectedAsset->GetPackages()[1].GetDescription().c_str(), actualAsset->GetPackages()[1].GetDescription().c_str());    
    }
