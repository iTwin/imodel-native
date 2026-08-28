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
* @bsi
+---------------+---------------+---------------+---------------+---------------+------*/
class VerticalDatumUnitTests : public ::testing::Test
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        VerticalDatumUnitTests() {};
        ~VerticalDatumUnitTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* Basic construction and behavior checks for VerticalTransform.
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, VerticalTransformBasicTest)
{
    Json::Value nullJson;
    nullJson["target"] = "WGS84";
    nullJson["nullTransform"] = Json::nullValue;

    GeoCoordinates::VerticalTransformPtr nullTransform =
        GeoCoordinates::VerticalTransform::CreateFromJson(nullJson, "NullTransformTest", "WGS84");
    ASSERT_TRUE(nullTransform.IsValid());
    EXPECT_EQ(nullTransform->GetTransformType(), GeoCoordinates::VerticalTransform::TransformType::Null);
    EXPECT_TRUE(0 == nullTransform->GetName().CompareToI("NullTransformTest"));
    EXPECT_TRUE(0 == nullTransform->GetTarget().CompareToI("WGS84"));

    GeoPoint point = {0.0, 0.0, 0.0};
    double elevationOffset = 0.0;
    GeoCoordinates::VerticalTransform::ElevationType elevationType = GeoCoordinates::VerticalTransform::ElevationType::Fixed;
    EXPECT_EQ(nullTransform->GetElevation(elevationOffset, elevationType, point), SUCCESS);
    EXPECT_NEAR(0.0, elevationOffset, 1.0e-12);
    EXPECT_EQ(elevationType, GeoCoordinates::VerticalTransform::ElevationType::Offset);

    Json::Value offsetJson;
    offsetJson["target"] = "NAVD88 height";
    offsetJson["verticalOffset"]["offset"] = 1.5;
    offsetJson["verticalOffset"]["units"] = "meter";

    GeoCoordinates::VerticalTransformPtr offsetTransform =
        GeoCoordinates::VerticalTransform::CreateFromJson(offsetJson, "OffsetTransformTest", "NAVD88 height");
    ASSERT_TRUE(offsetTransform.IsValid());
    EXPECT_EQ(offsetTransform->GetTransformType(), GeoCoordinates::VerticalTransform::TransformType::VerticalOffset);
    EXPECT_TRUE(0 == offsetTransform->GetName().CompareToI("OffsetTransformTest"));
    EXPECT_TRUE(0 == offsetTransform->GetTarget().CompareToI("NAVD88 height"));

    EXPECT_EQ(offsetTransform->GetElevation(elevationOffset, elevationType, point), SUCCESS);
    EXPECT_NEAR(1.5, elevationOffset, 1.0e-12);
    EXPECT_EQ(elevationType, GeoCoordinates::VerticalTransform::ElevationType::Offset);

    GeoCoordinates::VerticalTransformPtr reverseOffset = offsetTransform->CreateReverseCopy();
    ASSERT_TRUE(reverseOffset.IsValid());
    EXPECT_TRUE(0 == reverseOffset->GetName().CompareToI("NAVD88 height"));
    EXPECT_TRUE(0 == reverseOffset->GetTarget().CompareToI("OffsetTransformTest"));
    EXPECT_EQ(reverseOffset->GetElevation(elevationOffset, elevationType, point), SUCCESS);
    EXPECT_NEAR(-1.5, elevationOffset, 1.0e-12);

    EXPECT_TRUE(offsetTransform->IsEqualTo(*offsetTransform.get()));

    Json::Value offsetJsonDifferent;
    offsetJsonDifferent["target"] = "NAVD88 height";
    offsetJsonDifferent["verticalOffset"]["offset"] = 2.5;
    offsetJsonDifferent["verticalOffset"]["units"] = "meter";

    GeoCoordinates::VerticalTransformPtr differentOffsetTransform =
        GeoCoordinates::VerticalTransform::CreateFromJson(offsetJsonDifferent, "OffsetTransformTest", "NAVD88 height");
    ASSERT_TRUE(differentOffsetTransform.IsValid());
    EXPECT_FALSE(offsetTransform->IsEqualTo(*differentOffsetTransform.get()));
}

/*---------------------------------------------------------------------------------**//**
* A geoid transform backed by a real geoid grid file should initialize and calculate an offset.
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, VerticalTransformGeoidGridFileTest)
{
    Json::Value geoidJson;
    geoidJson["target"] = "WGS84";
    geoidJson["geoidSeparationGrid"]["direction"] = "Direct";
    geoidJson["geoidSeparationGrid"]["format"] = "GRD";
    geoidJson["geoidSeparationGrid"]["files"] = Json::arrayValue;
    geoidJson["geoidSeparationGrid"]["files"].append("./World/WW15MGH.GRD");

    GeoCoordinates::VerticalTransformPtr geoidTransform =
        GeoCoordinates::VerticalTransform::CreateFromJson(geoidJson, "EGM96 height2", "WGS84");
    ASSERT_TRUE(geoidTransform.IsValid());
    EXPECT_EQ(geoidTransform->GetTransformType(), GeoCoordinates::VerticalTransform::TransformType::GeoidSeparationGrid);
    EXPECT_EQ(geoidTransform->GetRequiredHorizontalDatumBase(), "WGS84");

    GeoPoint point = { 23.700523, 37.944210, 0.0 };
    double elevationOffset = 0.0;
    GeoCoordinates::VerticalTransform::ElevationType elevationType = GeoCoordinates::VerticalTransform::ElevationType::Fixed;
    StatusInt status = geoidTransform->GetElevation(elevationOffset, elevationType, point);
    EXPECT_EQ(status, SUCCESS);
    EXPECT_EQ(elevationType, GeoCoordinates::VerticalTransform::ElevationType::Offset);
    EXPECT_NEAR(elevationOffset, 38.3, 0.5);
}

/*---------------------------------------------------------------------------------**//**
* A geoid transform with a missing grid file should fail at GetElevation() time.
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, VerticalTransformGeoidGridFileMissingTest)
{
    Json::Value missingGeoidJson;
    missingGeoidJson["target"] = "WGS84";
    missingGeoidJson["geoidSeparationGrid"]["direction"] = "Direct";
    missingGeoidJson["geoidSeparationGrid"]["format"] = "GRD";
    missingGeoidJson["geoidSeparationGrid"]["files"] = Json::arrayValue;
    missingGeoidJson["geoidSeparationGrid"]["files"].append("./World/DefinitelyDoesNotExist.GRD");

    GeoCoordinates::VerticalTransformPtr missingGeoidTransform =
    GeoCoordinates::VerticalTransform::CreateFromJson(missingGeoidJson, "MissingGridTransform", "WGS84");
    ASSERT_TRUE(missingGeoidTransform.IsValid());

    GeoPoint point = { 0.0, 0.0, 0.0 };
    double elevationOffset = 0.0;
    GeoCoordinates::VerticalTransform::ElevationType elevationType = GeoCoordinates::VerticalTransform::ElevationType::Fixed;
    StatusInt status = missingGeoidTransform->GetElevation(elevationOffset, elevationType, point);
    EXPECT_NE(status, SUCCESS);
    EXPECT_EQ(status, GeoCoordinates::GEOCOORDERR_GeoCoordNotInitialized);
}

/*---------------------------------------------------------------------------------**//**
* Basic construction and API checks for VerticalTransformPathInfo.
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, VerticalTransformPathInfoBasicTest)
{
    Json::Value pathJson;
    pathJson["target"] = "WGS84";
    pathJson["path"] = Json::arrayValue;
    pathJson["path"].append("NAVD88 height");
    pathJson["path"].append("EGM96 height");
    pathJson["path"].append("WGS84");

    GeoCoordinates::VerticalTransformPathInfoPtr pathInfo =
        GeoCoordinates::VerticalTransformPathInfo::CreateFromJson(pathJson, "PathInfoTest");
    ASSERT_TRUE(pathInfo.IsValid());
    EXPECT_TRUE(0 == pathInfo->GetName().CompareToI("PathInfoTest"));
    EXPECT_TRUE(0 == pathInfo->GetTarget().CompareToI("WGS84"));

    bvector<Utf8String> path;
    pathInfo->GetPath(path);
    ASSERT_EQ(path.size(), 3U);
    EXPECT_TRUE(0 == path[0].CompareToI("NAVD88 height"));
    EXPECT_TRUE(0 == path[1].CompareToI("EGM96 height"));
    EXPECT_TRUE(0 == path[2].CompareToI("WGS84"));

    Json::Value roundTripJson;
    EXPECT_EQ(pathInfo->ToJson(roundTripJson), SUCCESS);
    EXPECT_TRUE(roundTripJson.isMember("target"));
    EXPECT_TRUE(roundTripJson.isMember("path"));
    EXPECT_TRUE(roundTripJson["path"].isArray());
    EXPECT_EQ(roundTripJson["path"].size(), 3U);

    GeoCoordinates::VerticalTransformPathInfoPtr samePathInfo =
        GeoCoordinates::VerticalTransformPathInfo::CreateFromJson(roundTripJson, "PathInfoTest");
    ASSERT_TRUE(samePathInfo.IsValid());
    EXPECT_TRUE(*pathInfo == *samePathInfo);

    Json::Value differentPathJson = pathJson;
    differentPathJson["path"][0] = "NAVD88 height";
    differentPathJson["path"][1] = "NGVD29 height";
    differentPathJson["path"][2] = "WGS84";
    GeoCoordinates::VerticalTransformPathInfoPtr differentPathInfo =
    GeoCoordinates::VerticalTransformPathInfo::CreateFromJson(differentPathJson, "PathInfoTest");
    ASSERT_TRUE(differentPathInfo.IsValid());
    EXPECT_FALSE(*pathInfo == *differentPathInfo);
}

/*---------------------------------------------------------------------------------**//**
* Tests that FromVerticalJson and SetVerticalDatumFromJson expect different JSON
* structures: FromVerticalJson takes the vertical datum content directly (without a
* "verticalCRS" wrapper), whereas SetVerticalDatumFromJson expects the JSON to have an
* englobing "verticalCRS" node.
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, VerticalDatumFromJsonVsSetFromJsonWrapperTest)
{
    // Build the raw vertical datum content (no "verticalCRS" wrapper)
    Json::Value datumContent;
    datumContent["crsName"]   = "TestGeoid";
    datumContent["datumName"] = "TestGeoidDatum";
    datumContent["type"]      = "GEOID";
    datumContent["units"]     = "Meter";
    datumContent["extent"]["southWest"]["latitude"]  = -90.0;
    datumContent["extent"]["southWest"]["longitude"] = -180.0;
    datumContent["extent"]["northEast"]["latitude"]  =  90.0;
    datumContent["extent"]["northEast"]["longitude"] =  180.0;
    datumContent["transforms"][0]["target"]       = "WGS84";
    datumContent["transforms"][0]["nullTransform"] = Json::nullValue;

    // Build the "verticalCRS"-wrapped version expected by SetVerticalDatumFromJson
    Json::Value wrappedJson;
    wrappedJson["verticalCRS"] = datumContent;

    GeoCoordinates::BaseGCSPtr gcs = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    ASSERT_TRUE(gcs.IsValid());

    // --- FromVerticalJson ---
    // Succeeds with raw content (no "verticalCRS" outer node)
    Utf8String errorMessage;
    EXPECT_EQ(gcs->FromVerticalJson(datumContent, errorMessage), SUCCESS);
    EXPECT_TRUE(gcs->HasValidVerticalDatum());

    // Fails when given the wrapped JSON: the "verticalCRS" key is not a recognised datum field
    GeoCoordinates::BaseGCSPtr gcs2 = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    ASSERT_TRUE(gcs2.IsValid());
    EXPECT_NE(gcs2->FromVerticalJson(wrappedJson, errorMessage), SUCCESS);

    // --- SetVerticalDatumFromJson ---
    // Succeeds when given the wrapped JSON (with the "verticalCRS" outer node)
    GeoCoordinates::BaseGCSPtr gcs3 = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    ASSERT_TRUE(gcs3.IsValid());
    EXPECT_EQ(gcs3->SetVerticalDatumFromJson(wrappedJson), SUCCESS);
    EXPECT_TRUE(gcs3->HasValidVerticalDatum());

    // Fails when given the raw content directly (missing "verticalCRS" wrapper)
    GeoCoordinates::BaseGCSPtr gcs4 = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    ASSERT_TRUE(gcs4.IsValid());
    EXPECT_NE(gcs4->SetVerticalDatumFromJson(datumContent), SUCCESS);
}

/*---------------------------------------------------------------------------------**//**
* BaseGCS::FromVerticalJson accepts incomplete JSON definitions when they specify
* one of the supported minimal identifiers: crsName, epsg, or id.
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, FromVerticalJsonAcceptsIncompleteDefinitionsByIdentifier)
{
    Utf8String errorMessage;

    // Resolve a known vertical CRS and EPSG from the dictionary to avoid hard-coding.
    GeoCoordinates::BaseGCSPtr seedGcs = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    ASSERT_TRUE(seedGcs.IsValid());
    ASSERT_EQ(seedGcs->SetVerticalDatumFromName("WGS84"), SUCCESS);

    Json::Value knownVerticalJson;
    ASSERT_EQ(seedGcs->ToVerticalJson(knownVerticalJson), SUCCESS);
    ASSERT_TRUE(knownVerticalJson.isMember("crsName"));
    ASSERT_TRUE(knownVerticalJson.isMember("epsg"));

    Utf8String knownCrsName = knownVerticalJson["crsName"].asString();
    int knownEpsg = knownVerticalJson["epsg"].asInt();
    ASSERT_TRUE(knownCrsName.length() > 0);
    ASSERT_TRUE(knownEpsg > 0);

    // Case 1: only crsName
    {
    Json::Value verticalJson;
    verticalJson["crsName"] = knownCrsName;

    GeoCoordinates::BaseGCSPtr gcs = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    ASSERT_TRUE(gcs.IsValid());
    EXPECT_EQ(gcs->FromVerticalJson(verticalJson, errorMessage), SUCCESS);
    EXPECT_TRUE(gcs->HasValidVerticalDatum());
    }

    // Case 2: only epsg
    {
    Json::Value verticalJson;
    verticalJson["epsg"] = knownEpsg;

    GeoCoordinates::BaseGCSPtr gcs = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    ASSERT_TRUE(gcs.IsValid());
    EXPECT_EQ(gcs->FromVerticalJson(verticalJson, errorMessage), SUCCESS);
    EXPECT_TRUE(gcs->HasValidVerticalDatum());
    }

    // Case 3: only legacy id
    {
    Json::Value verticalJson;
    verticalJson["id"] = "ELLIPSOID";

    GeoCoordinates::BaseGCSPtr gcs = GeoCoordinates::BaseGCS::CreateGCS("LL84");
    ASSERT_TRUE(gcs.IsValid());
    EXPECT_EQ(gcs->FromVerticalJson(verticalJson, errorMessage), SUCCESS);
    EXPECT_EQ(gcs->GetNetVerticalDatumCode(), GeoCoordinates::vdcEllipsoid);
    }
}

/*---------------------------------------------------------------------------------**//**
* VerticalDatumInfo::CreateFromJson does not accept incomplete JSON definitions when
* mandatory properties are absent, even if crsName/epsg/id are present.
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, VerticalDatumInfoCreateFromJsonRejectsIncompleteDefinitions)
{
    // Only crsName
    {
    Json::Value jsonValue;
    jsonValue["crsName"] = "WGS84";

    StatusInt status = SUCCESS;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(jsonValue, false, status);

    EXPECT_FALSE(datumInfo.IsValid());
    EXPECT_EQ(status, GeoCoordinates::GEOCOORDERR_UnknownDatumType);
    }

    // Only epsg
    {
    Json::Value jsonValue;
    jsonValue["epsg"] = 4979;

    StatusInt status = SUCCESS;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(jsonValue, false, status);

    EXPECT_FALSE(datumInfo.IsValid());
    EXPECT_EQ(status, GeoCoordinates::GEOCOORDERR_NoCRSName);
    }

    // Only id (legacy property used by BaseGCS::FromVerticalJson, not by VerticalDatumInfo)
    {
    Json::Value jsonValue;
    jsonValue["id"] = "ELLIPSOID";

    StatusInt status = SUCCESS;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(jsonValue, false, status);

    EXPECT_FALSE(datumInfo.IsValid());
    EXPECT_EQ(status, GeoCoordinates::GEOCOORDERR_NoCRSName);
    }
}

/*---------------------------------------------------------------------------------**//**
* A vertical datum created from JSON should serialize back to equivalent JSON.
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, VerticalDatumInfoJsonRoundTripTest)
{
    Json::Value inputJson;
    inputJson["crsName"] = "TestRoundTripDatum";
    inputJson["datumName"] = "Test Round Trip Datum";
    inputJson["epsg"] = 12345;
    inputJson["type"] = "GEOID";
    inputJson["description"] = "Round trip test datum";
    inputJson["areaOfUse"] = "Test area";
    inputJson["remarks"] = "Round trip remarks";
    inputJson["units"] = "meter";
    inputJson["deprecated"] = true;
    inputJson["extent"]["southWest"]["latitude"] = -10.0;
    inputJson["extent"]["southWest"]["longitude"] = -20.0;
    inputJson["extent"]["northEast"]["latitude"] = 10.0;
    inputJson["extent"]["northEast"]["longitude"] = 20.0;
    inputJson["transforms"][0]["target"] = "WGS84";
    inputJson["transforms"][0]["nullTransform"] = Json::nullValue;
    inputJson["transformPaths"][0]["target"] = "WGS84";
    inputJson["transformPaths"][0]["path"] = Json::arrayValue;
    inputJson["transformPaths"][0]["path"].append("TestRoundTripDatum");
    inputJson["transformPaths"][0]["path"].append("WGS84");

    StatusInt status = ERROR;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(inputJson, false, status);
    ASSERT_TRUE(datumInfo.IsValid());
    EXPECT_EQ(status, SUCCESS);

    Json::Value serializedJson;
    EXPECT_EQ(datumInfo->ToJson(serializedJson), SUCCESS);

    EXPECT_EQ(serializedJson["crsName"].asString(), inputJson["crsName"].asString());
    EXPECT_EQ(serializedJson["datumName"].asString(), inputJson["datumName"].asString());
    EXPECT_EQ(serializedJson["epsg"].asInt(), inputJson["epsg"].asInt());
    EXPECT_EQ(serializedJson["type"].asString(), inputJson["type"].asString());
    EXPECT_EQ(serializedJson["description"].asString(), inputJson["description"].asString());
    EXPECT_EQ(serializedJson["areaOfUse"].asString(), inputJson["areaOfUse"].asString());
    EXPECT_EQ(serializedJson["remarks"].asString(), inputJson["remarks"].asString());
    EXPECT_EQ(serializedJson["units"].asString(), inputJson["units"].asString());
    EXPECT_EQ(serializedJson["deprecated"].asBool(), inputJson["deprecated"].asBool());

    EXPECT_DOUBLE_EQ(serializedJson["extent"]["southWest"]["latitude"].asDouble(),
        inputJson["extent"]["southWest"]["latitude"].asDouble());
    EXPECT_DOUBLE_EQ(serializedJson["extent"]["southWest"]["longitude"].asDouble(),
        inputJson["extent"]["southWest"]["longitude"].asDouble());
    EXPECT_DOUBLE_EQ(serializedJson["extent"]["northEast"]["latitude"].asDouble(),
        inputJson["extent"]["northEast"]["latitude"].asDouble());
    EXPECT_DOUBLE_EQ(serializedJson["extent"]["northEast"]["longitude"].asDouble(),
        inputJson["extent"]["northEast"]["longitude"].asDouble());

    EXPECT_EQ(serializedJson["transforms"].size(), inputJson["transforms"].size());
    EXPECT_EQ(serializedJson["transforms"][0]["target"].asString(), inputJson["transforms"][0]["target"].asString());
    EXPECT_EQ(serializedJson["transformPaths"][0]["target"].asString(), inputJson["transformPaths"][0]["target"].asString());
    EXPECT_EQ(serializedJson["transformPaths"][0]["path"][0].asString(), inputJson["transformPaths"][0]["path"][0].asString());
    EXPECT_EQ(serializedJson["transformPaths"][0]["path"][1].asString(), inputJson["transformPaths"][0]["path"][1].asString());
}

/*---------------------------------------------------------------------------------**//**
* After creating a VerticalDatumInfo from JSON, each accessor method must return the
* value that was provided in the input JSON.
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, VerticalDatumInfoGettersTest)
{
    Json::Value inputJson;
    inputJson["crsName"]    = "GetterTestDatum";
    inputJson["datumName"]  = "Getter Test Datum Name";
    inputJson["epsg"]       = 9999;
    inputJson["type"]       = "GEOID";
    inputJson["description"]= "Getter test description";
    inputJson["areaOfUse"]  = "Getter test area";
    inputJson["remarks"]    = "Getter test remarks";
    inputJson["units"]      = "Meter";
    inputJson["deprecated"] = true;
    inputJson["extent"]["southWest"]["latitude"]  = -45.0;
    inputJson["extent"]["southWest"]["longitude"] = -90.0;
    inputJson["extent"]["northEast"]["latitude"]  =  45.0;
    inputJson["extent"]["northEast"]["longitude"] =  90.0;
    inputJson["transforms"][0]["target"]        = "WGS84";
    inputJson["transforms"][0]["nullTransform"] = Json::nullValue;
    inputJson["transforms"][1]["target"]        = "EGM96 height";
    inputJson["transforms"][1]["verticalOffset"]["offset"] = 2.5;
    inputJson["transforms"][1]["verticalOffset"]["units"]  = "Meter";

    StatusInt status = ERROR;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(inputJson, false, status);
    ASSERT_TRUE(datumInfo.IsValid());
    ASSERT_EQ(status, SUCCESS);

    // --- string accessors ---
    Utf8String crsName;
    datumInfo->GetCRSName(crsName);
    EXPECT_EQ(crsName, "GetterTestDatum");

    Utf8String datumName;
    datumInfo->GetDatumName(datumName);
    EXPECT_EQ(datumName, "Getter Test Datum Name");

    Utf8String type;
    datumInfo->GetType(type);
    EXPECT_EQ(type, "GEOID");

    Utf8String description;
    datumInfo->GetDescription(description);
    EXPECT_EQ(description, "Getter test description");

    Utf8String areaOfUse;
    datumInfo->GetAreaOfUse(areaOfUse);
    EXPECT_EQ(areaOfUse, "Getter test area");

    Utf8String remarks;
    datumInfo->GetRemarks(remarks);
    EXPECT_EQ(remarks, "Getter test remarks");

    Utf8String units;
    datumInfo->GetUnits(units);
    EXPECT_TRUE(0 == units.CompareToI("Meter"));

    // --- numeric/bool accessors ---
    EXPECT_TRUE(datumInfo->EPSGCodeIsValid());
    EXPECT_EQ(datumInfo->GetEPSGCode(), 9999);

    EXPECT_TRUE(datumInfo->IsDeprecated());

    // UnitsFromMeter(): "Meter" -> factor 1.0
    EXPECT_NEAR(datumInfo->UnitsFromMeter(), 1.0, 1.0e-12);

    // --- extent ---
    DRange2d extent;
    datumInfo->GetExtent(extent);
    EXPECT_NEAR(extent.low.y,  -45.0, 1.0e-12);   // southWest latitude
    EXPECT_NEAR(extent.low.x,  -90.0, 1.0e-12);   // southWest longitude
    EXPECT_NEAR(extent.high.y,  45.0, 1.0e-12);   // northEast latitude
    EXPECT_NEAR(extent.high.x,  90.0, 1.0e-12);   // northEast longitude

    // --- transform target names ---
    bvector<Utf8String> targetNames;
    EXPECT_EQ(datumInfo->GetTransformTargetNames(targetNames), SUCCESS);
    ASSERT_EQ(targetNames.size(), 2U);
    EXPECT_TRUE(0 == targetNames[0].CompareToI("WGS84"));
    EXPECT_TRUE(0 == targetNames[1].CompareToI("EGM96 height"));
}

/*---------------------------------------------------------------------------------**//**
* VerticalDatumInfo::CreateFromJson must return GEOCOORDERR_NoTransforms and a null
* pointer when the JSON contains a transforms entry that is structurally invalid:
*   - a transform object that is missing the mandatory "target" field
*   - a transforms array entry that is not a JSON object at all
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, VerticalDatumInfoInvalidTransformTest)
{
    // Build the valid outer part of the datum JSON (reused for every sub-case)
    auto buildBaseDatum = [](Json::Value& json)
        {
        json["crsName"]   = "InvalidTransformDatum";
        json["datumName"] = "Invalid Transform Datum";
        json["type"]      = "GEOID";
        json["units"]     = "Meter";
        json["extent"]["southWest"]["latitude"]  = -90.0;
        json["extent"]["southWest"]["longitude"] = -180.0;
        json["extent"]["northEast"]["latitude"]  =  90.0;
        json["extent"]["northEast"]["longitude"] =  180.0;
        };

    // --- Case 1: transform object is missing the "target" field ---
    {
    Json::Value datumJson;
    buildBaseDatum(datumJson);
    datumJson["transforms"][0]["nullTransform"] = Json::nullValue;
    // Note: "target" is intentionally absent

    StatusInt status = SUCCESS;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(datumJson, false, status);

    EXPECT_FALSE(datumInfo.IsValid());
    EXPECT_EQ(status, GeoCoordinates::GEOCOORDERR_NoTransforms);
    }

    // --- Case 2: transforms array entry is not an object (a bare string) ---
    {
    Json::Value datumJson;
    buildBaseDatum(datumJson);
    datumJson["transforms"].append("this is not a transform object");

    StatusInt status = SUCCESS;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(datumJson, false, status);

    EXPECT_FALSE(datumInfo.IsValid());
    EXPECT_EQ(status, GeoCoordinates::GEOCOORDERR_NoTransforms);
    }

    // --- Sanity: a valid transform succeeds ---
    {
    Json::Value datumJson;
    buildBaseDatum(datumJson);
    datumJson["transforms"][0]["target"]        = "WGS84";
    datumJson["transforms"][0]["nullTransform"] = Json::nullValue;

    StatusInt status = ERROR;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(datumJson, false, status);

    EXPECT_TRUE(datumInfo.IsValid());
    EXPECT_EQ(status, SUCCESS);
    }
}

/*---------------------------------------------------------------------------------**//**
* VerticalDatumInfo::CreateFromJson must fail with GEOCOORDERR_InvalidTransformPath
* when transform path constraints are violated:
*   - path target must match the last path entry
*   - first path entry must match the owning vertical datum name
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(VerticalDatumUnitTests, VerticalDatumInfoInvalidTransformPathTest)
{
    auto buildBaseDatum = [](Json::Value& json)
        {
        json["crsName"]   = "PathValidationDatum";
        json["datumName"] = "Path Validation Datum";
        json["type"]      = "GEOID";
        json["units"]     = "Meter";
        json["extent"]["southWest"]["latitude"]  = -90.0;
        json["extent"]["southWest"]["longitude"] = -180.0;
        json["extent"]["northEast"]["latitude"]  =  90.0;
        json["extent"]["northEast"]["longitude"] =  180.0;
        json["transforms"][0]["target"]        = "WGS84";
        json["transforms"][0]["nullTransform"] = Json::nullValue;
        };

    // Case 1: transformPath target differs from last path entry.
    {
    Json::Value jsonValue;
    buildBaseDatum(jsonValue);
    jsonValue["transformPaths"][0]["target"] = "WGS84";
    jsonValue["transformPaths"][0]["path"] = Json::arrayValue;
    jsonValue["transformPaths"][0]["path"].append("PathValidationDatum");
    jsonValue["transformPaths"][0]["path"].append("EGM96 height"); // last entry intentionally mismatches target

    StatusInt status = SUCCESS;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(jsonValue, false, status);

    EXPECT_FALSE(datumInfo.IsValid());
    EXPECT_EQ(status, GeoCoordinates::GEOCOORDERR_InvalidTransformPath);
    }

    // Case 2: first path entry differs from owning vertical datum (crsName).
    {
    Json::Value jsonValue;
    buildBaseDatum(jsonValue);
    jsonValue["transformPaths"][0]["target"] = "WGS84";
    jsonValue["transformPaths"][0]["path"] = Json::arrayValue;
    jsonValue["transformPaths"][0]["path"].append("SomeOtherDatum"); // first entry intentionally invalid
    jsonValue["transformPaths"][0]["path"].append("WGS84");

    StatusInt status = SUCCESS;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(jsonValue, false, status);

    EXPECT_FALSE(datumInfo.IsValid());
    EXPECT_EQ(status, GeoCoordinates::GEOCOORDERR_InvalidTransformPath);
    }

    // Sanity: valid transform path should be accepted.
    {
    Json::Value jsonValue;
    buildBaseDatum(jsonValue);
    jsonValue["transformPaths"][0]["target"] = "WGS84";
    jsonValue["transformPaths"][0]["path"] = Json::arrayValue;
    jsonValue["transformPaths"][0]["path"].append("PathValidationDatum");
    jsonValue["transformPaths"][0]["path"].append("WGS84");

    StatusInt status = ERROR;
    GeoCoordinates::VerticalDatumInfoPtr datumInfo =
        GeoCoordinates::VerticalDatumInfo::CreateFromJson(jsonValue, false, status);

    EXPECT_TRUE(datumInfo.IsValid());
    EXPECT_EQ(status, SUCCESS);
    }
}


/*---------------------------------------------------------------------------------**//**
* @bsi                                                   Sarah.Keenan   09/2024
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalQueryPointTest
    {
    GeoPoint2d          m_testPt;
    StatusInt           m_testStatus;
    int                 m_minNumberAvailableVerticalDatums;
    bvector<Utf8String>    m_expectedFoundVerticalDatums;
    };
 
// Preparation of required environment
class VerticalDatumUnitTestsQueryPoint : public ::testing::TestWithParam< verticalQueryPointTest >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        VerticalDatumUnitTestsQueryPoint() {};
        ~VerticalDatumUnitTestsQueryPoint() {};
    };

static bvector<verticalQueryPointTest> s_listOfPointTests = 
    {
        // {pt long, lat}, status, minimum number of applicable vertical datums found, some named datums expected to be found
        { {0.0, 0.0},       SUCCESS, 2, {"WGS84","EGM96 height"} },
        { {0.0, -90.0},     SUCCESS, 2, {"WGS84","EGM96 height"} },
        { {0.0, 90.0},      SUCCESS, 2, {"WGS84","EGM96 height"} },
        { {-180.0, 0.0},    SUCCESS, 2, {"WGS84","EGM96 height"} },
        { {180.0, 0.0},     SUCCESS, 2, {"WGS84","EGM96 height"} },
        { {0.0, -90.1},     GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
        { {0.0, 90.1},      GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
        { {-180.1, 0.0},    GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
        { {180.1, 0.0},     GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
        { {178.16, 34.34},  SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
        { {-79.0, 42.0},    SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
        { {178.16, 42.0},   SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
        { {-79.0, 34.34},   SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
    };


/*---------------------------------------------------------------------------------**//**
* Query GeoCoord to find Vertical Datums that are availble for a specific position
* lat/long
* @bsimethod                                                    Sarah.Keenan  2024-07
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(VerticalDatumUnitTestsQueryPoint, VerticalDatumPointUnitTest)
{
    verticalQueryPointTest theTestParam = GetParam(); 

    GeoCoordinates::BaseGCSPtr testGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    bvector<Utf8String> verticalDatums;
    StatusInt status = testGCS->QueryVerticalDatumsAvailableAtPoint(verticalDatums, theTestParam.m_testPt.longitude, theTestParam.m_testPt.latitude);
    ASSERT_TRUE(theTestParam.m_testStatus == status);
    ASSERT_TRUE(theTestParam.m_minNumberAvailableVerticalDatums <= verticalDatums.size());
    int numFound = 0;
    for (const auto& testDatumName : theTestParam.m_expectedFoundVerticalDatums)
    {
        for (const auto& datumName : verticalDatums)
        {
            if (0 == datumName.CompareToI(testDatumName))
            {
                numFound++;
                break;
            }
        }
    }
    ASSERT_TRUE(numFound == theTestParam.m_expectedFoundVerticalDatums.size());
}

/*---------------------------------------------------------------------------------**//**
* @bsi                                                   Sarah.Keenan   09/2024
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalQueryExtentTest
{
    GeoPoint2d          m_testMinExtent;
    GeoPoint2d          m_testMaxExtent;
    StatusInt           m_testStatus;
    int                 m_minNumberAvailableVerticalDatums;
    bvector<Utf8String>    m_expectedFoundVerticalDatums;
};

// Preparation of required environment
class VerticalDatumUnitTestsQueryExtent : public ::testing::TestWithParam< verticalQueryExtentTest >
{   
public:
    virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
    virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

    VerticalDatumUnitTestsQueryExtent() {};
    ~VerticalDatumUnitTestsQueryExtent() {};
};

static bvector<verticalQueryExtentTest> s_listOfExtentTests = 
{
    // {range long min, lat min}, {range long max, lat max}, status, minimum number of applicable vertical datums found, a named datum expected to be found
    { {0.0, 0.0},       {0.0, 0.0},         SUCCESS, 2, {"WGS84","EGM96 height"} },
    { {0.0, -90.0},     {0.0, 0.0},         SUCCESS, 2, {"WGS84","EGM96 height"} },
    { {0.0, 0.0},       {0.0, 90.0},        SUCCESS, 2, {"WGS84","EGM96 height"} },
    { {-180.0, 0.0},    {0.0, 0.0},         SUCCESS, 2, {"WGS84","EGM96 height"} },
    { {0.0, 0.0},       {180.0, 0.0},       SUCCESS, 2, {"WGS84","EGM96 height"} },
    { {0.0, -90.1},     {0.0, 0.0},         GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
    { {0.0, 0.0},       {0.0, 90.1},        GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
    { {-180.1, 0.0},    {0.0, 0.0},         GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
    { {0.0, 0.0},       {180.1, 0.0},       GeoCoordinates::GEOCOORDERR_CoordinateRange, 0 },
    { {178.10, 34.30},  {178.20, 34.40},    SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
    { {-79.0, 42.0},    {-78.0, 43.0},      SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
    { {178.16, 42.0},   {179.16, 41.0},     SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
    { {-79.1, 34.30},   {-79.0, 34.40},     SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },
    { {178.16, 42.0},   {-179.16, 41.0},    SUCCESS, 4, {"WGS84","EGM96 height","NGVD29 height","NAVD88 height"} },

};

/*---------------------------------------------------------------------------------**//**
* Query GeoCoord to find Vertical Datums that are availble for a specific range
* lat/long
* @bsimethod                                                    Sarah.Keenan  2024-07
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(VerticalDatumUnitTestsQueryExtent, VerticalDatumUnitTestsExtent)
{
    verticalQueryExtentTest theTestParam = GetParam(); 

    GeoCoordinates::BaseGCSPtr testGCS = GeoCoordinates::BaseGCS::CreateGCS("LL84");

    bvector<Utf8String> verticalDatums;
    StatusInt status = testGCS->QueryVerticalDatumsAvailableForRange(verticalDatums, theTestParam.m_testMinExtent.longitude, theTestParam.m_testMinExtent.latitude,
                                                                    theTestParam.m_testMaxExtent.longitude, theTestParam.m_testMaxExtent.latitude, false);
    ASSERT_TRUE(theTestParam.m_testStatus == status);
    ASSERT_TRUE(theTestParam.m_minNumberAvailableVerticalDatums <= verticalDatums.size());
    int numFound = 0;
    for (const auto& testDatumName : theTestParam.m_expectedFoundVerticalDatums)
    {
        for (const auto& datumName : verticalDatums)
        {
            if (0 == datumName.CompareToI(testDatumName))
            {
                numFound++;
                break;
            }
        }
    }
    ASSERT_TRUE(numFound == theTestParam.m_expectedFoundVerticalDatums.size());

}

/*---------------------------------------------------------------------------------**//**
* @bsi                                                   Sarah.Keenan   09/2024
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalDatumEqualTest
{
    Utf8String     m_verticalDatumName1;
    Utf8String     m_verticalDatumName2;
    bool        m_equal;
};

// Preparation of required environment
class VerticalDatumUnitTestEquivalence : public ::testing::TestWithParam< verticalDatumEqualTest >
{   
public:
    virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
    virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

    VerticalDatumUnitTestEquivalence() {};
    ~VerticalDatumUnitTestEquivalence() {};
};

static bvector<verticalDatumEqualTest> s_listOfVerticalDatumEqualTests = 
{
    // {vertical datum name in dictionary}, {name of vertical datum to compare with}, vertical datums are equal
    // Note: the reason for all the equal items is to test the operator==()/IsEqualTo() methods
    // for VerticalDatum, VerticalDatumInfo, VerticalTransform and VerticalTransformPathInfo
    { "WGS84",                 "WGS84",           true },
    { "EGM96 height",          "EGM96 height",    true },
    { "EGM2008 height",        "EGM2008 height",  true },
    { "NAVD88 height",         "NAVD88 height",   true },
    { "NGVD29 height",         "NGVD29 height",   true },
    { "Kiunga",                "Kiunga",          true },
    { "NAVD88 height",         "NGVD29 height",   false },
};

/*---------------------------------------------------------------------------------**//**
* Equivalence test
* @bsimethod                                                    Sarah.Keenan  2024-07
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(VerticalDatumUnitTestEquivalence, VerticalDatumUnitTestEqual)
{
    verticalDatumEqualTest theTestParam = GetParam(); 

    StatusInt status1, status2;
    GeoCoordinates::VerticalDatumPtr verticalDatum1 = GeoCoordinates::BaseGCS::CreateVerticalDatumFromName(theTestParam.m_verticalDatumName1.c_str(), status1);
    GeoCoordinates::VerticalDatumPtr verticalDatum2 = GeoCoordinates::BaseGCS::CreateVerticalDatumFromName(theTestParam.m_verticalDatumName2.c_str(), status2);

    ASSERT_TRUE(verticalDatum1.IsValid());
    ASSERT_TRUE(verticalDatum2.IsValid());
    ASSERT_TRUE(theTestParam.m_equal == (*(verticalDatum1.get()) == *(verticalDatum2.get())));
}

/*---------------------------------------------------------------------------------**//**
* @bsi                                                   Sarah.Keenan   09/2024
+---------------+---------------+---------------+---------------+---------------+------*/
struct verticalDatumInitFromNameTest
{
    Utf8String     m_verticalDatumName;
    Utf8String     m_crsName;
    bool        m_canInitializeUsingName;
};

// Preparation of required environment
class VerticalDatumUnitTestInitFromName : public ::testing::TestWithParam< verticalDatumInitFromNameTest >
{   
public:
    virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
    virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

    VerticalDatumUnitTestInitFromName() {};
    ~VerticalDatumUnitTestInitFromName() {};
};

static bvector<verticalDatumInitFromNameTest> s_listOfVerticalDatumInitFromNameTests = 
{
    // crs names not in dictionary, legacy names, can create new dictionary vertical datum if equivalent available
    { "LOCAL_ELLIPSOID", "BritishNatGrid", false },
    { "ELLIPSOID", "LL84", true },    // Creates 'WGS84'
    { "GEOID", "LL84", false },
    { "NAVD88", "CT83", true },       // Creates 'NAVD88 height'
    { "NGVD29", "CT83", true },       // Creates 'NGVD29 height'
    // crs names in dictionary, should be able to create new dictionary vertical datum
    { "WGS84", "LL84", true },
    { "NAVD88 height", "LL84", true },
    { "NAVD88(Geoid12b) height", "LL84", true },
    { "NGVD29 height", "LL84", true },
    { "EGM96 height", "LL84", true },
    { "EGM2008 height", "LL84", true },
    { "Kiunga", "LL84", true },
    { "CGVD28 height", "LL84", true },
    { "CGVD2013(CGG2013a) height", "LL84", true },
    { "OSGM15", "LL84", true },
    { "NAP2018", "LL84", true },
    { "OSGM02", "LL84", true }
};

/*---------------------------------------------------------------------------------**//**
* Set vertical datum using Name test vs set using legacy key, the legacy keys should never
* be available in the vertical datum dictionary so setting by name should fail for those
* @bsimethod                                                    Sarah.Keenan  2024-11
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(VerticalDatumUnitTestInitFromName, VerticalDatumUnitTestInitFromName)
{
    verticalDatumInitFromNameTest theTestParam = GetParam(); 

    // init using vertical datum dictionary
    GeoCoordinates::BaseGCSPtr gcs1 = GeoCoordinates::BaseGCS::CreateGCS(theTestParam.m_crsName.c_str());
    gcs1->SetVerticalDatumFromName(theTestParam.m_verticalDatumName.c_str());
    ASSERT_TRUE(theTestParam.m_canInitializeUsingName == gcs1->HasValidVerticalDatum());
}

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumUnitTestsQueryPoint,
                         ValuesIn(s_listOfPointTests));

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumUnitTestsQueryExtent,
                         ValuesIn(s_listOfExtentTests));

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumUnitTestEquivalence,
                         ValuesIn(s_listOfVerticalDatumEqualTests));

INSTANTIATE_TEST_SUITE_P(SpecificTransformTests_Combined,
                         VerticalDatumUnitTestInitFromName,
                         ValuesIn(s_listOfVerticalDatumInitFromNameTests));
