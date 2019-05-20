//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST


#include <Bentley/BeTest.h>
#include <Bentley/BeTextFile.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/SpatialEntity.h>
#include <RealityPlatform/RealityDataObjects.h>

#include <algorithm>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
// @bsimethod                          Alain.Robert                            02/2017
//=====================================================================================
class RealityDataObjectTestFixture : public testing::Test
    {
    public:
        WCharCP GetDirectory()
            {
            BeFileName outDir;
            BeTest::GetHost().GetTempDir (outDir);
            return outDir;
            }

        static Json::Value s_SampleRealityDataFolderJSON;

        static Json::Value GetSampleRealityDataFolderJSON()
            {
            Utf8CP jsonString = "{"
                "\"instances\": ["
                "{"
                "\"instanceId\": \"43a4a51a-bfd3-4271-a9d9-21db56cdcf10~2FScene\","
                "\"schemaName\": \"S3MX\","
                "\"className\": \"Folder\","
                "\"properties\": {"
                "\"Name\": \"Scene\","
                "\"RealityDataId\": \"43a4a51a-bfd3-4271-a9d9-21db56cdcf10\","
                "\"ParentFolderId\": \"43a4a51a-bfd3-4271-a9d9-21db56cdcf10/\""
                "},"
                "\"eTag\": \"agKs8UGYbn244uJSSjBZp+nt8wo=\""
                "}"
                "]"
                "}";


            // Parse.
            Json::Value root(Json::objectValue);
            Json::Reader::Parse(jsonString, root);
            return root["instances"][0];
            }
    };

Json::Value RealityDataObjectTestFixture::s_SampleRealityDataFolderJSON = GetSampleRealityDataFolderJSON();
//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityBasicTest)
    {

    // Creation of a spatial entity
    SpatialEntityPtr mySpatialEntity = SpatialEntity::Create();

    ASSERT_TRUE(mySpatialEntity.IsValid());
    ASSERT_TRUE(!mySpatialEntity.IsNull());

    // Check default values
    EXPECT_STREQ(mySpatialEntity->GetIdentifier().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetName().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetResolution().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetProvider().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetProviderName().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetDataType().c_str(), "");
    EXPECT_TRUE(mySpatialEntity->GetClassification() == SpatialEntity::Classification::UNDEFINED_CLASSIF);
    ASSERT_STRCASEEQ(mySpatialEntity->GetClassificationTag().c_str(), "Undefined"); // Default is preset
    EXPECT_STREQ(mySpatialEntity->GetDataset().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetThumbnailURL().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetThumbnailLoginKey().c_str(), "");
    EXPECT_TRUE(mySpatialEntity->GetApproximateFileSize() == 0);
    EXPECT_TRUE(!mySpatialEntity->GetDate().IsValid()); // Time not set should be invalid
    EXPECT_TRUE(mySpatialEntity->GetFootprint().size() == 0);
    EXPECT_TRUE(!mySpatialEntity->HasApproximateFootprint());
    DRange2d myRange = mySpatialEntity->GetFootprintExtent();
    EXPECT_TRUE(myRange.low.x == 0.0);
    EXPECT_TRUE(myRange.high.x == 0.0);
    EXPECT_TRUE(myRange.low.y == 0.0);
    EXPECT_TRUE(myRange.high.y == 0.0);


    EXPECT_TRUE(mySpatialEntity->GetDataSourceCount() == 0);
    EXPECT_STREQ(mySpatialEntity->GetDescription().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetAccuracy().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetVisibilityTag().c_str(), "UNDEFINED"); // Default is preset

    EXPECT_TRUE(mySpatialEntity->GetMetadataCP() == NULL);
    EXPECT_TRUE(mySpatialEntity->GetOcclusion() == 0.0); // Default value indicates invalid

    // Check set methods
    mySpatialEntity->SetIdentifier("f28fdab2-c369-4913-b18a-fbe541af635c");
    EXPECT_STREQ(mySpatialEntity->GetIdentifier().c_str(), "f28fdab2-c369-4913-b18a-fbe541af635c");

    mySpatialEntity->SetName("NameOfItem");
    EXPECT_STREQ(mySpatialEntity->GetName().c_str(), "NameOfItem");

    mySpatialEntity->SetResolution("13.4x15.4");
    EXPECT_STREQ(mySpatialEntity->GetResolution().c_str(), "13.4x15.4");
    EXPECT_NEAR(mySpatialEntity->GetResolutionValue(), 14.36, 0.01);

    mySpatialEntity->SetProvider("Provider1");
    EXPECT_STREQ(mySpatialEntity->GetProvider().c_str(), "Provider1");
    mySpatialEntity->SetProviderName("ProviderName1");
    EXPECT_STREQ(mySpatialEntity->GetProviderName().c_str(), "ProviderName1");
    mySpatialEntity->SetDataType("tif;jpg");
    EXPECT_STREQ(mySpatialEntity->GetDataType().c_str(), "tif;jpg");
    mySpatialEntity->SetClassification(SpatialEntity::Classification::MODEL);
    ASSERT_TRUE(mySpatialEntity->GetClassification() == SpatialEntity::Classification::MODEL);
    ASSERT_STRCASEEQ(mySpatialEntity->GetClassificationTag().c_str(), "Model");
    mySpatialEntity->SetDataset("MyDataset1");
    EXPECT_STREQ(mySpatialEntity->GetDataset().c_str(), "MyDataset1");
    mySpatialEntity->SetThumbnailURL("http:\\example.com\thumbnail.jpg");
    EXPECT_STREQ(mySpatialEntity->GetThumbnailURL().c_str(), "http:\\example.com\thumbnail.jpg");
    mySpatialEntity->SetThumbnailLoginKey("BentleyCONNECT");
    EXPECT_STREQ(mySpatialEntity->GetThumbnailLoginKey().c_str(), "BentleyCONNECT");
    mySpatialEntity->SetApproximateFileSize(123765473);
    EXPECT_TRUE(mySpatialEntity->GetApproximateFileSize() == 123765473);
    mySpatialEntity->SetDate(DateTime(2017,02,27));
    EXPECT_TRUE(mySpatialEntity->GetDate().IsValid());
    EXPECT_TRUE(mySpatialEntity->GetDate() == DateTime(2017,02,27));

    bvector<GeoPoint2d> myFootprint;
    myFootprint.push_back(GeoPoint2d::From(12.5, 45.8));
    myFootprint.push_back(GeoPoint2d::From(12.5, 46.8));
    myFootprint.push_back(GeoPoint2d::From(13.5, 46.8));
    myFootprint.push_back(GeoPoint2d::From(13.5, 45.8));
    myFootprint.push_back(GeoPoint2d::From(12.5, 45.8));

    mySpatialEntity->SetFootprint(myFootprint);
    EXPECT_TRUE(mySpatialEntity->GetFootprint().size() == 5);
    mySpatialEntity->SetApproximateFootprint(true);
    EXPECT_TRUE(mySpatialEntity->HasApproximateFootprint());

    myRange = mySpatialEntity->GetFootprintExtent();
    EXPECT_TRUE(myRange.low.x == 12.5);
    EXPECT_TRUE(myRange.high.x == 13.5);
    EXPECT_TRUE(myRange.low.y == 45.8);
    EXPECT_TRUE(myRange.high.y == 46.8);

    EXPECT_STREQ(mySpatialEntity->GetFootprintString().c_str(), "{\\\"points\\\":[[12.500000,45.800000],[12.500000,46.800000],[13.500000,46.800000],[13.500000,45.800000],[12.500000,45.800000]], \\\"coordinate_system\\\":\\\"4326\\\"}");

    mySpatialEntity->SetFootprintString("{\\\"points\\\":[0,0],[1,0],[1,1],[0,1],[0,0]], \\\"coordinate_system\\\":\\\"4326\\\"}");
    EXPECT_STREQ(mySpatialEntity->GetFootprintString().c_str(),"{\\\"points\\\":[0,0],[1,0],[1,1],[0,1],[0,0]], \\\"coordinate_system\\\":\\\"4326\\\"}");

    EXPECT_TRUE(mySpatialEntity->GetDataSourceCount() == 0);

    mySpatialEntity->SetDescription("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt");
    EXPECT_STREQ(mySpatialEntity->GetDescription().c_str(), "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt");
    mySpatialEntity->SetAccuracy("2.34");
    EXPECT_STREQ(mySpatialEntity->GetAccuracy().c_str(), "2.34");
    mySpatialEntity->SetVisibilityByTag("ENTERPRISE");
    EXPECT_STREQ(mySpatialEntity->GetVisibilityTag().c_str(), "ENTERPRISE"); // Default is preset

    EXPECT_TRUE(mySpatialEntity->GetMetadataCP() == NULL);

    mySpatialEntity->SetOcclusion(23.45);
    EXPECT_NEAR(mySpatialEntity->GetOcclusion(), 23.45, 0.00001);

    mySpatialEntity->SetResolution("15.225");
    EXPECT_NEAR(mySpatialEntity->GetResolutionValue(), 15.225, 0.00001);

    mySpatialEntity->SetResolution("");
    EXPECT_NEAR(mySpatialEntity->GetResolutionValue(), 0.0, 0.00001);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityMetadataBasicTest)
    {

    // Creation of a spatial entity
    SpatialEntityMetadataPtr mySpatialEntityMetadata = SpatialEntityMetadata::Create();

    ASSERT_TRUE(mySpatialEntityMetadata.IsValid());
    ASSERT_TRUE(!mySpatialEntityMetadata.IsNull());

    ASSERT_TRUE(mySpatialEntityMetadata->IsEmpty());

    EXPECT_STREQ(mySpatialEntityMetadata->GetId().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetProvenance().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetLineage().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetDescription().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetContactInfo().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetLegal().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetTermsOfUse().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetKeywords().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetFormat().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetMetadataUrl().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetDisplayStyle().c_str(), "");

    // Set
    ASSERT_TRUE(mySpatialEntityMetadata->IsEmpty());
    mySpatialEntityMetadata->SetProvenance("ProvenanceText");
    EXPECT_STREQ(mySpatialEntityMetadata->GetProvenance().c_str(), "ProvenanceText");
    ASSERT_TRUE(!mySpatialEntityMetadata->IsEmpty());

    mySpatialEntityMetadata->SetLineage("LineageText");
    EXPECT_STREQ(mySpatialEntityMetadata->GetLineage().c_str(), "LineageText");

    mySpatialEntityMetadata->SetDescription("DescriptionText");
    EXPECT_STREQ(mySpatialEntityMetadata->GetDescription().c_str(), "DescriptionText");

    mySpatialEntityMetadata->SetContactInfo("ContactInfoText");
    EXPECT_STREQ(mySpatialEntityMetadata->GetContactInfo().c_str(), "ContactInfoText");

    mySpatialEntityMetadata->SetLegal("LegalText");
    EXPECT_STREQ(mySpatialEntityMetadata->GetLegal().c_str(), "LegalText");

    mySpatialEntityMetadata->SetTermsOfUse("TermsOfUseText");
    EXPECT_STREQ(mySpatialEntityMetadata->GetTermsOfUse().c_str(), "TermsOfUseText");

    mySpatialEntityMetadata->SetKeywords("Keyword1;Keyword2;Keyword3");
    EXPECT_STREQ(mySpatialEntityMetadata->GetKeywords().c_str(), "Keyword1;Keyword2;Keyword3");

    mySpatialEntityMetadata->SetFormat("FGDC");
    EXPECT_STREQ(mySpatialEntityMetadata->GetFormat().c_str(), "FGDC");

    mySpatialEntityMetadata->SetMetadataUrl("http:\\\\somewhere.com\\metadata.xml");
    EXPECT_STREQ(mySpatialEntityMetadata->GetMetadataUrl().c_str(), "http:\\\\somewhere.com\\metadata.xml");

    mySpatialEntityMetadata->SetId("MyID");
    EXPECT_STREQ(mySpatialEntityMetadata->GetId().c_str(), "MyID");

    mySpatialEntityMetadata->SetDisplayStyle("MyDisplayStyle");
    EXPECT_STREQ(mySpatialEntityMetadata->GetDisplayStyle().c_str(), "MyDisplayStyle");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                       Remi.Charbonneau                            05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityMetadataCreateFromMetadataCR)
    {
    SpatialEntityMetadataPtr mySpatialEntityMetadata = SpatialEntityMetadata::Create();
    mySpatialEntityMetadata->SetProvenance("ProvenanceText");
    mySpatialEntityMetadata->SetLineage("LineageText");
    mySpatialEntityMetadata->SetDescription("DescriptionText");
    mySpatialEntityMetadata->SetContactInfo("ContactInfoText");
    mySpatialEntityMetadata->SetLegal("LegalText");
    mySpatialEntityMetadata->SetTermsOfUse("TermsOfUseText");
    mySpatialEntityMetadata->SetKeywords("Keyword1;Keyword2;Keyword3");
    mySpatialEntityMetadata->SetFormat("FGDC");
    mySpatialEntityMetadata->SetMetadataUrl("http:\\\\somewhere.com\\metadata.xml");
    mySpatialEntityMetadata->SetId("MyID");
    mySpatialEntityMetadata->SetDisplayStyle("MyDisplayStyle");

    SpatialEntityMetadataPtr secondSpatialEntityMetadata = SpatialEntityMetadata::CreateFromMetadata(*mySpatialEntityMetadata);
    EXPECT_STREQ(secondSpatialEntityMetadata->GetProvenance().c_str(), "ProvenanceText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetLineage().c_str(), "LineageText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetDescription().c_str(), "DescriptionText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetContactInfo().c_str(), "ContactInfoText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetLegal().c_str(), "LegalText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetTermsOfUse().c_str(), "TermsOfUseText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetKeywords().c_str(), "Keyword1;Keyword2;Keyword3");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetFormat().c_str(), "FGDC");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetMetadataUrl().c_str(), "http:\\\\somewhere.com\\metadata.xml");
    //ID is the only value not copied from the seed
    EXPECT_STREQ(secondSpatialEntityMetadata->GetId().c_str(), "");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetDisplayStyle().c_str(), "MyDisplayStyle");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                       Remi.Charbonneau                            05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityMetadataCreateFromFile)
    {
    SpatialEntityMetadataPtr mySpatialEntityMetadata = SpatialEntityMetadata::Create();
    mySpatialEntityMetadata->SetProvenance("ProvenanceText");
    mySpatialEntityMetadata->SetLineage("LineageText");
    mySpatialEntityMetadata->SetDescription("DescriptionText");
    mySpatialEntityMetadata->SetContactInfo("ContactInfoText");
    mySpatialEntityMetadata->SetLegal("LegalText");
    mySpatialEntityMetadata->SetTermsOfUse("TermsOfUseText");
    mySpatialEntityMetadata->SetKeywords("Keyword1;Keyword2;Keyword3");
    mySpatialEntityMetadata->SetFormat("FGDC");
    mySpatialEntityMetadata->SetMetadataUrl("http:\\\\somewhere.com\\metadata.xml");
    mySpatialEntityMetadata->SetId("MyID");
    mySpatialEntityMetadata->SetDisplayStyle("MyDisplayStyle");

    // Create XML Files
    BeFileName fileName(GetDirectory());
    fileName.AppendToPath(L"xmlTestfile.xml");
    BeFileStatus status;
    BeTextFilePtr tempFile = BeTextFile::Open(status, fileName.c_str(), TextFileOpenType::Write, TextFileOptions::None);
    tempFile->PutLine(L"<xml><root><entitee /></root></xml>",false);
    tempFile->Close();

    Utf8String fileName2(fileName.c_str());

    SpatialEntityMetadataPtr secondSpatialEntityMetadata = SpatialEntityMetadata::CreateFromFile(fileName2.c_str(), *mySpatialEntityMetadata);

    EXPECT_STREQ(secondSpatialEntityMetadata->GetProvenance().c_str(), "xmlTestfile.xml");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetLineage().c_str(), "LineageText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetDescription().c_str(), "DescriptionText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetContactInfo().c_str(), "ContactInfoText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetLegal().c_str(), "LegalText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetTermsOfUse().c_str(), "TermsOfUseText");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetKeywords().c_str(), "Keyword1;Keyword2;Keyword3");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetFormat().c_str(), "xml");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetMetadataUrl().c_str(), "http:\\\\somewhere.com\\metadata.xml");
    //ID is the only value not copied from the seed
    EXPECT_STREQ(secondSpatialEntityMetadata->GetId().c_str(), "");
    EXPECT_STREQ(secondSpatialEntityMetadata->GetDisplayStyle().c_str(), "MyDisplayStyle");

    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityDataSourceBasicTest)
    {
    // Creation of a spatial entity
    SpatialEntityDataSourcePtr mySpatialEntityDataSource = SpatialEntityDataSource::Create();

    ASSERT_TRUE(mySpatialEntityDataSource.IsValid());
    ASSERT_TRUE(!mySpatialEntityDataSource.IsNull());

    EXPECT_STREQ(mySpatialEntityDataSource->GetUri().ToString().c_str(), "");

    EXPECT_STREQ(mySpatialEntityDataSource->GetGeoCS().c_str(), "");

    EXPECT_STREQ(mySpatialEntityDataSource->GetCompoundType().c_str(), "");

    EXPECT_TRUE(mySpatialEntityDataSource->GetSize() == 0);

    EXPECT_STREQ(mySpatialEntityDataSource->GetNoDataValue().c_str(), "");

    EXPECT_STREQ(mySpatialEntityDataSource->GetDataType().c_str(), "");

    EXPECT_STREQ(mySpatialEntityDataSource->GetLocationInCompound().c_str(), "");

    //ASSERT_TRUE(!mySpatialEntityDataSource->GetIsMultiband());

    /*Utf8String redUrl;
    Utf8String greenUrl;
    Utf8String blueUrl;
    Utf8String panchromaticUrl;

    mySpatialEntityDataSource->GetMultibandUrls(redUrl, greenUrl, blueUrl, panchromaticUrl);
    EXPECT_STREQ(redUrl.c_str(), "");
    EXPECT_STREQ(greenUrl.c_str(), "");
    EXPECT_STREQ(blueUrl.c_str(), "");
    EXPECT_STREQ(panchromaticUrl.c_str(), "");

    ASSERT_TRUE(mySpatialEntityDataSource->GetRedBandSize() == 0);

    ASSERT_TRUE(mySpatialEntityDataSource->GetGreenBandSize() == 0);

    ASSERT_TRUE(mySpatialEntityDataSource->GetBlueBandSize() == 0);

    ASSERT_TRUE(mySpatialEntityDataSource->GetPanchromaticBandSize() == 0);*/

    // Set test
    UriPtr uri = Uri::Create("http:\\\\somewhere.com\\SomeLocation");
    mySpatialEntityDataSource->SetUri(*uri);
    EXPECT_STREQ(mySpatialEntityDataSource->GetUri().ToString().c_str(), "http:\\\\somewhere.com\\SomeLocation");

    mySpatialEntityDataSource->SetGeoCS("EPSG:4326;NAVD88");
    EXPECT_STREQ(mySpatialEntityDataSource->GetGeoCS().c_str(), "EPSG:4326;NAVD88");

    mySpatialEntityDataSource->SetCompoundType("zip");
    EXPECT_STREQ(mySpatialEntityDataSource->GetCompoundType().c_str(), "zip");

    mySpatialEntityDataSource->SetSize(12345);
    EXPECT_TRUE(mySpatialEntityDataSource->GetSize() == 12345);

    mySpatialEntityDataSource->SetNoDataValue("0:0:0");
    EXPECT_STREQ(mySpatialEntityDataSource->GetNoDataValue().c_str(), "0:0:0");

    mySpatialEntityDataSource->SetDataType("tif");
    EXPECT_STREQ(mySpatialEntityDataSource->GetDataType().c_str(), "tif");

    mySpatialEntityDataSource->SetLocationInCompound(".\\a.tif");
    EXPECT_STREQ(mySpatialEntityDataSource->GetLocationInCompound().c_str(), ".\\a.tif");

    /*mySpatialEntityDataSource->SetIsMultiband(true);
    ASSERT_TRUE(mySpatialEntityDataSource->GetIsMultiband());

    mySpatialEntityDataSource->SetMultibandUrls("red.tif", "green.tif", "blue.tif", "pan.tif");
    mySpatialEntityDataSource->GetMultibandUrls(redUrl, greenUrl, blueUrl, panchromaticUrl);
    EXPECT_STREQ(redUrl.c_str(), "red.tif");
    EXPECT_STREQ(greenUrl.c_str(), "green.tif");
    EXPECT_STREQ(blueUrl.c_str(), "blue.tif");
    EXPECT_STREQ(panchromaticUrl.c_str(), "pan.tif");

    mySpatialEntityDataSource->SetRedBandSize(324);
    ASSERT_TRUE(mySpatialEntityDataSource->GetRedBandSize() == 324);

    mySpatialEntityDataSource->SetGreenBandSize(321);
    ASSERT_TRUE(mySpatialEntityDataSource->GetGreenBandSize() == 321);

    mySpatialEntityDataSource->SetBlueBandSize(453);
    ASSERT_TRUE(mySpatialEntityDataSource->GetBlueBandSize() == 453);

    mySpatialEntityDataSource->SetPanchromaticBandSize(2345);
    ASSERT_TRUE(mySpatialEntityDataSource->GetPanchromaticBandSize() == 2345);*/
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityServerBasicTest)
    {
    SpatialEntityServerPtr mySpatialEntityServer = SpatialEntityServer::Create();

    ASSERT_TRUE(mySpatialEntityServer.IsValid());
    ASSERT_TRUE(!mySpatialEntityServer.IsNull());

    EXPECT_STREQ(mySpatialEntityServer->GetProtocol().c_str(), "");
    EXPECT_STREQ(mySpatialEntityServer->GetType().c_str(), "");
    EXPECT_STREQ(mySpatialEntityServer->GetName().c_str(), "");
    EXPECT_STREQ(mySpatialEntityServer->GetUrl().c_str(), "");
    EXPECT_STREQ(mySpatialEntityServer->GetContactInfo().c_str(), "");
    EXPECT_STREQ(mySpatialEntityServer->GetLegal().c_str(), "");
    EXPECT_TRUE(mySpatialEntityServer->IsOnline());
    EXPECT_TRUE(!mySpatialEntityServer->GetLastCheck().IsValid());
    EXPECT_TRUE(!mySpatialEntityServer->GetLastTimeOnline().IsValid());
    EXPECT_TRUE(mySpatialEntityServer->GetLatency() == 0.0);
    EXPECT_STREQ(mySpatialEntityServer->GetState().c_str(), "");
    EXPECT_TRUE(!mySpatialEntityServer->IsStreamed());
    EXPECT_STREQ(mySpatialEntityServer->GetLoginKey().c_str(), "");
    EXPECT_STREQ(mySpatialEntityServer->GetLoginMethod().c_str(), "");
    EXPECT_STREQ(mySpatialEntityServer->GetRegistrationPage().c_str(), "");
    EXPECT_STREQ(mySpatialEntityServer->GetOrganisationPage().c_str(), "");
    EXPECT_TRUE(mySpatialEntityServer->GetMeanReachabilityStats() == 0);
    EXPECT_STREQ(mySpatialEntityServer->GetFees().c_str(), "");
    EXPECT_STREQ(mySpatialEntityServer->GetAccessConstraints().c_str(), "");


    // Set tests
    mySpatialEntityServer->SetProtocol("http");
    EXPECT_STREQ(mySpatialEntityServer->GetProtocol().c_str(), "http");

    mySpatialEntityServer->SetType("WMS");
    EXPECT_STREQ(mySpatialEntityServer->GetType().c_str(), "WMS");

    mySpatialEntityServer->SetName("OneWMSServer");
    EXPECT_STREQ(mySpatialEntityServer->GetName().c_str(), "OneWMSServer");

    mySpatialEntityServer->SetUrl("http:\\\\wms.server.com\\serverPath\\");
    EXPECT_STREQ(mySpatialEntityServer->GetUrl().c_str(), "http:\\\\wms.server.com\\serverPath\\");

    mySpatialEntityServer->SetContactInfo("toto@wms.server.com");
    EXPECT_STREQ(mySpatialEntityServer->GetContactInfo().c_str(), "toto@wms.server.com");

    mySpatialEntityServer->SetLegal("Copyright of OneWMS Server 2016: Data can be accessed freely. See details as http:\\\\wms.server.com\\copyright.html");
    EXPECT_STREQ(mySpatialEntityServer->GetLegal().c_str(), "Copyright of OneWMS Server 2016: Data can be accessed freely. See details as http:\\\\wms.server.com\\copyright.html");

    mySpatialEntityServer->SetOnline(false);
    EXPECT_FALSE(mySpatialEntityServer->IsOnline());

    mySpatialEntityServer->SetLastCheck(DateTime(2017,02,17));
    EXPECT_TRUE(mySpatialEntityServer->GetLastCheck() == DateTime(2017,02,17));

    mySpatialEntityServer->SetLastTimeOnline(DateTime(2017,02,17));
    EXPECT_TRUE(mySpatialEntityServer->GetLastTimeOnline() == DateTime(2017,02,17));

    mySpatialEntityServer->SetLatency(12.4);
    EXPECT_TRUE(mySpatialEntityServer->GetLatency() == 12.4);

    mySpatialEntityServer->SetState("MyState");
    EXPECT_STREQ(mySpatialEntityServer->GetState().c_str(), "MyState");

    mySpatialEntityServer->SetStreamed(true);
    EXPECT_TRUE(mySpatialEntityServer->IsStreamed());

    mySpatialEntityServer->SetLoginKey("BentleyCONNECT");
    EXPECT_STREQ(mySpatialEntityServer->GetLoginKey().c_str(), "BentleyCONNECT");

    mySpatialEntityServer->SetLoginMethod("CUSTOM");
    EXPECT_STREQ(mySpatialEntityServer->GetLoginMethod().c_str(), "CUSTOM");

    mySpatialEntityServer->SetRegistrationPage("http:\\\\wwww.example.com");
    EXPECT_STREQ(mySpatialEntityServer->GetRegistrationPage().c_str(), "http:\\\\wwww.example.com");

    mySpatialEntityServer->SetOrganisationPage("http:\\\\wwww.example.com/organisation.html");
    EXPECT_STREQ(mySpatialEntityServer->GetOrganisationPage().c_str(), "http:\\\\wwww.example.com/organisation.html");

    mySpatialEntityServer->SetFees("somefees");
    EXPECT_STREQ(mySpatialEntityServer->GetFees().c_str(), "somefees");

    mySpatialEntityServer->SetAccessConstraints("constraints");
    EXPECT_STREQ(mySpatialEntityServer->GetAccessConstraints().c_str(), "constraints");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityServerCreateFromURLAndName)
    {

    SpatialEntityServerPtr mySpatialEntityServer = SpatialEntityServer::Create("http://www.example.com/myurl.html", "MyServerName");

    ASSERT_TRUE(mySpatialEntityServer.IsValid());
    ASSERT_TRUE(!mySpatialEntityServer.IsNull());

    ASSERT_STREQ(mySpatialEntityServer->GetUrl().c_str(), "http://www.example.com/myurl.html");
    ASSERT_STREQ(mySpatialEntityServer->GetName().c_str(), "MyServerName");
    ASSERT_TRUE(mySpatialEntityServer->IsOnline());
    ASSERT_FALSE(mySpatialEntityServer->IsStreamed());
    ASSERT_STREQ(mySpatialEntityServer->GetProtocol().c_str(), "http");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityServerCreateFromURLOnly)
    {
    auto date = DateTime::GetCurrentTimeUtc();
    SpatialEntityServerPtr mySpatialEntityServer = SpatialEntityServer::Create("http://www.example.com");

    ASSERT_TRUE(mySpatialEntityServer.IsValid());
    ASSERT_TRUE(!mySpatialEntityServer.IsNull());

    ASSERT_STREQ(mySpatialEntityServer->GetUrl().c_str(), "http://www.example.com");
    ASSERT_STREQ(mySpatialEntityServer->GetName().c_str(), "www.example.com");
    ASSERT_TRUE(mySpatialEntityServer->IsOnline());
    ASSERT_FALSE(mySpatialEntityServer->IsStreamed());
    ASSERT_STREQ(mySpatialEntityServer->GetProtocol().c_str(), "http");
    ASSERT_TRUE(DateTime::Compare(mySpatialEntityServer->GetLastCheck(), date) == DateTime::CompareResult::LaterThan ||
                DateTime::Compare(mySpatialEntityServer->GetLastCheck(), date) == DateTime::CompareResult::Equals);
    ASSERT_TRUE(DateTime::Compare(mySpatialEntityServer->GetLastTimeOnline(), date) == DateTime::CompareResult::LaterThan ||
                DateTime::Compare(mySpatialEntityServer->GetLastTimeOnline(), date) == DateTime::CompareResult::Equals);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityCompleteTest)
    {
    SpatialEntityPtr mySpatialEntity = SpatialEntity::Create();

    mySpatialEntity->SetIdentifier("f28fdab2-c369-4913-b18a-fbe541af635c");
    mySpatialEntity->SetName("NameOfItem");
    mySpatialEntity->SetResolution("13.4x15.4");
    mySpatialEntity->SetProvider("Provider1");
    mySpatialEntity->SetProviderName("ProviderName1");
    mySpatialEntity->SetDataType("tif;jpg");
    mySpatialEntity->SetClassification(SpatialEntity::Classification::MODEL);
    mySpatialEntity->SetDataset("MyDataset1");
    mySpatialEntity->SetThumbnailURL("http:\\example.com\thumbnail.jpg");
    mySpatialEntity->SetThumbnailLoginKey("BentleyCONNECT");
    mySpatialEntity->SetApproximateFileSize(123765473);
    mySpatialEntity->SetDate(DateTime(2017,02,27));

    bvector<GeoPoint2d> myFootprint;
    myFootprint.push_back(GeoPoint2d::From(12.5, 45.8));
    myFootprint.push_back(GeoPoint2d::From(12.5, 46.8));
    myFootprint.push_back(GeoPoint2d::From(13.5, 46.8));
    myFootprint.push_back(GeoPoint2d::From(13.5, 45.8));
    myFootprint.push_back(GeoPoint2d::From(12.5, 45.8));

    mySpatialEntity->SetFootprint(myFootprint);
    mySpatialEntity->SetApproximateFootprint(true);

    DRange2d myRange = mySpatialEntity->GetFootprintExtent();
    EXPECT_TRUE(myRange.low.x == 12.5);
    EXPECT_TRUE(myRange.high.x == 13.5);
    EXPECT_TRUE(myRange.low.y == 45.8);
    EXPECT_TRUE(myRange.high.y == 46.8);

    mySpatialEntity->SetDescription("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt");
    mySpatialEntity->SetAccuracy("2.34");
    mySpatialEntity->SetVisibilityByTag("ENTERPRISE");

    mySpatialEntity->SetOcclusion(23.45);





    // Create and add a spatial data source
    SpatialEntityDataSourcePtr mySpatialEntityDataSource = SpatialEntityDataSource::Create();

    UriPtr uri = Uri::Create("http:\\\\somewhere.com\\SomeLocation");
    mySpatialEntityDataSource->SetUri(*uri);
    mySpatialEntityDataSource->SetGeoCS("EPSG:4326;NAVD88");
    mySpatialEntityDataSource->SetCompoundType("zip");
    mySpatialEntityDataSource->SetSize(12345);
    mySpatialEntityDataSource->SetNoDataValue("0:0:0");
    mySpatialEntityDataSource->SetDataType("tif");
    mySpatialEntityDataSource->SetLocationInCompound(".\\a.tif");
    /*mySpatialEntityDataSource->SetIsMultiband(true);
    mySpatialEntityDataSource->SetMultibandUrls("red.tif", "green.tif", "blue.tif", "pan.tif");
    mySpatialEntityDataSource->SetRedBandSize(324);
    mySpatialEntityDataSource->SetGreenBandSize(321);
    mySpatialEntityDataSource->SetBlueBandSize(453);
    mySpatialEntityDataSource->SetPanchromaticBandSize(2345);*/

    // Create a server.
    SpatialEntityServerPtr mySpatialEntityServer = SpatialEntityServer::Create();
    mySpatialEntityServer->SetProtocol("http");
    mySpatialEntityServer->SetType("WMS");
    mySpatialEntityServer->SetName("OneWMSServer");
    mySpatialEntityServer->SetUrl("http:\\\\wms.server.com\\serverPath\\");
    mySpatialEntityServer->SetContactInfo("toto@wms.server.com");
    mySpatialEntityServer->SetLegal("Copyright of OneWMS Server 2016: Data can be accessed freely. See details as http:\\\\wms.server.com\\copyright.html");
    mySpatialEntityServer->SetOnline(true);
    mySpatialEntityServer->SetLastCheck(DateTime(2017,02,17));
    mySpatialEntityServer->SetLastTimeOnline(DateTime(2017,02,17));
    mySpatialEntityServer->SetLatency(12.4);
    mySpatialEntityServer->SetState("MyState");
    mySpatialEntityServer->SetStreamed(true);
    mySpatialEntityServer->SetLoginKey("BentleyCONNECT");
    mySpatialEntityServer->SetLoginMethod("CUSTOM");

    // Add server to data source
    mySpatialEntityDataSource->SetServer(mySpatialEntityServer.get());
    ASSERT_TRUE(mySpatialEntityDataSource->GetServerCP() != NULL);

    // Add the first spatial data source
    mySpatialEntity->AddDataSource(*mySpatialEntityDataSource);
    EXPECT_TRUE(mySpatialEntity->GetDataSourceCount() == 1);

    // Create metadata
    SpatialEntityMetadataPtr mySpatialEntityMetadata = SpatialEntityMetadata::Create();

    mySpatialEntityMetadata->SetProvenance("ProvenanceText");
    mySpatialEntityMetadata->SetLineage("LineageText");
    mySpatialEntityMetadata->SetDescription("DescriptionText");
    mySpatialEntityMetadata->SetContactInfo("ContactInfoText");
    mySpatialEntityMetadata->SetLegal("LegalText");
    mySpatialEntityMetadata->SetTermsOfUse("TermsOfUseText");
    mySpatialEntityMetadata->SetKeywords("Keyword1;Keyword2;Keyword3");
    mySpatialEntityMetadata->SetFormat("FGDC");
    mySpatialEntityMetadata->SetMetadataUrl("http:\\\\somewhere.com\\metadata.xml");

    // Add metadata to spatial entity
    mySpatialEntity->SetMetadata(mySpatialEntityMetadata.get());

    EXPECT_TRUE(mySpatialEntity->GetMetadataCP() != NULL);

    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityConstructorWithParams)
    {
    bvector<GeoPoint2d> inputfootPrint;
    inputfootPrint.push_back(GeoPoint2d::From(12.5, 45.8));
    inputfootPrint.push_back(GeoPoint2d::From(12.5, 46.8));
    inputfootPrint.push_back(GeoPoint2d::From(13.5, 46.8));
    inputfootPrint.push_back(GeoPoint2d::From(13.5, 45.8));
    inputfootPrint.push_back(GeoPoint2d::From(12.5, 45.8));
    auto creationTime = DateTime(2017,02,27);
    SpatialEntityPtr mySpatialEntity = SpatialEntity::Create("MyIdentifier", creationTime, "13.4x15.4", inputfootPrint, "MyName" , "9999");

    EXPECT_STREQ(mySpatialEntity->GetIdentifier().c_str(), "MyIdentifier");
    EXPECT_TRUE(DateTime::Compare(mySpatialEntity->GetDate(), creationTime) == DateTime::CompareResult::Equals);
    EXPECT_STREQ(mySpatialEntity->GetResolution().c_str(), "13.4x15.4");
    EXPECT_NEAR(mySpatialEntity->GetResolutionValue(), 14.36, 0.01);
    EXPECT_STREQ(mySpatialEntity->GetName().c_str(), "MyName");

    auto outputFootPrint = mySpatialEntity->GetFootprint();
    EXPECT_TRUE(std::equal(inputfootPrint.begin(), inputfootPrint.end(), outputFootPrint.begin(), [](GeoPoint2d left, GeoPoint2d right)
        {
        if (left.latitude == right.latitude && left.longitude == right.longitude)
            return true;
        return false;
        }));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataBasicTest)
    {

    // Creation of a spatial entity
    RealityDataPtr myRealityData = RealityData::Create();

    ASSERT_TRUE(myRealityData.IsValid());
    ASSERT_TRUE(!myRealityData.IsNull());

    // Check default values
    EXPECT_STREQ(myRealityData->GetIdentifier().c_str(), "");
    EXPECT_STREQ(myRealityData->GetName().c_str(), "");
    EXPECT_STREQ(myRealityData->GetResolution().c_str(), "");
    EXPECT_STREQ(myRealityData->GetRealityDataType().c_str(), "");
    EXPECT_TRUE(myRealityData->IsStreamed() == true);
    EXPECT_TRUE(myRealityData->GetClassification() == RealityData::Classification::UNDEFINED_CLASSIF);
    ASSERT_STRCASEEQ(myRealityData->GetClassificationTag().c_str(), "Undefined"); // Default is preset
    EXPECT_STREQ(myRealityData->GetDataset().c_str(), "");
    EXPECT_STREQ(myRealityData->GetThumbnailDocument().c_str(), "");
    EXPECT_TRUE(myRealityData->GetTotalSize() == 0);
    EXPECT_TRUE(myRealityData->IsSizeUpToDate());
    EXPECT_TRUE(!myRealityData->GetCreationDateTime().IsValid()); // Time not set should be invalid
    EXPECT_TRUE(myRealityData->GetFootprint().size() == 0);
    EXPECT_TRUE(!myRealityData->HasApproximateFootprint());
    DRange2d myRange = myRealityData->GetFootprintExtent();
    EXPECT_TRUE(myRange.low.x == 0.0);
    EXPECT_TRUE(myRange.high.x == 0.0);
    EXPECT_TRUE(myRange.low.y == 0.0);
    EXPECT_TRUE(myRange.high.y == 0.0);


    EXPECT_STREQ(myRealityData->GetOrganizationId().c_str(), "");
    EXPECT_STREQ(myRealityData->GetContainerName().c_str(), "");
    EXPECT_STREQ(myRealityData->GetDataLocationGuid().c_str(), "");
    EXPECT_STREQ(myRealityData->GetDescription().c_str(), "");
    EXPECT_STREQ(myRealityData->GetRootDocument().c_str(), "");
    EXPECT_STREQ(myRealityData->GetAccuracy().c_str(), "");
    EXPECT_STREQ(myRealityData->GetVisibilityTag().c_str(), "UNDEFINED"); // Default is preset
    EXPECT_TRUE(myRealityData->IsListable());
    EXPECT_STREQ(myRealityData->GetOwner().c_str(), "");
    EXPECT_STREQ(myRealityData->GetCreatorId().c_str(), "");
    EXPECT_FALSE(myRealityData->IsHidden());
    EXPECT_FALSE(myRealityData->HasDelegatePermissions());

    EXPECT_STREQ(myRealityData->GetMetadataUrl().c_str(), "");
    EXPECT_STREQ(myRealityData->GetUltimateId().c_str(), "");
    EXPECT_STREQ(myRealityData->GetUltimateSite().c_str(), "");
    EXPECT_STREQ(myRealityData->GetCopyright().c_str(), "");
    EXPECT_STREQ(myRealityData->GetTermsOfUse().c_str(), "");
    EXPECT_TRUE(!myRealityData->GetModifiedDateTime().IsValid()); // Time not set should be invalid
    EXPECT_TRUE(!myRealityData->GetLastAccessedDateTime().IsValid()); // Time not set should be invalid
    EXPECT_STREQ(myRealityData->GetGroup().c_str(), "");
    EXPECT_TRUE(!myRealityData->GetLastAccessedDateTime().IsValid());
    EXPECT_FALSE(myRealityData->HasApproximateFootprint());

    // Check set methods
    myRealityData->SetIdentifier("f28fdab2-c369-4913-b18a-fbe541af635c");
    EXPECT_STREQ(myRealityData->GetIdentifier().c_str(), "f28fdab2-c369-4913-b18a-fbe541af635c");

    myRealityData->SetName("NameOfItem");
    EXPECT_STREQ(myRealityData->GetName().c_str(), "NameOfItem");

    myRealityData->SetResolution("13.4x15.4");
    EXPECT_STREQ(myRealityData->GetResolution().c_str(), "13.4x15.4");
    EXPECT_NEAR(myRealityData->GetResolutionValue(), 14.36, 0.01);


    myRealityData->SetRealityDataType("3mx");
    EXPECT_STREQ(myRealityData->GetRealityDataType().c_str(), "3mx");
    myRealityData->SetStreamed(false);
    ASSERT_TRUE(myRealityData->IsStreamed() == false);
    myRealityData->SetClassification(RealityData::Classification::MODEL);
    ASSERT_TRUE(myRealityData->GetClassification() == RealityData::Classification::MODEL);
    ASSERT_STRCASEEQ(myRealityData->GetClassificationTag().c_str(), "Model");
    myRealityData->SetDataset("MyDataset1");
    EXPECT_STREQ(myRealityData->GetDataset().c_str(), "MyDataset1");
    myRealityData->SetThumbnailDocument("thumbnail.jpg");
    EXPECT_STREQ(myRealityData->GetThumbnailDocument().c_str(), "thumbnail.jpg");
    myRealityData->SetTotalSize(123765473);
    EXPECT_TRUE(myRealityData->GetTotalSize() == 123765473);
    myRealityData->SetSizeUpToDate(false);
    EXPECT_TRUE(!myRealityData->IsSizeUpToDate());
    myRealityData->SetCreationDateTime(DateTime(2017,02,27));
    EXPECT_TRUE(myRealityData->GetCreationDateTime().IsValid());
    EXPECT_TRUE(myRealityData->GetCreationDateTime() == DateTime(2017,02,27));

    bvector<GeoPoint2d> myFootprint;
    myFootprint.push_back(GeoPoint2d::From(12.5, 45.8));
    myFootprint.push_back(GeoPoint2d::From(12.5, 46.8));
    myFootprint.push_back(GeoPoint2d::From(13.5, 46.8));
    myFootprint.push_back(GeoPoint2d::From(13.5, 45.8));
    myFootprint.push_back(GeoPoint2d::From(12.5, 45.8));

    myRealityData->SetFootprint(myFootprint);
    EXPECT_TRUE(myRealityData->GetFootprint().size() == 5);
    myRealityData->SetApproximateFootprint(true);
    EXPECT_TRUE(myRealityData->HasApproximateFootprint());

    myRange = myRealityData->GetFootprintExtent();
    EXPECT_TRUE(myRange.low.x == 12.5);
    EXPECT_TRUE(myRange.high.x == 13.5);
    EXPECT_TRUE(myRange.low.y == 45.8);
    EXPECT_TRUE(myRange.high.y == 46.8);


    EXPECT_STREQ(myRealityData->GetFootprintString().c_str(), "{\"Coordinates\": [{\"Longitude\": \"12.500000000\", \"Latitude\": \"45.800000000\"},{\"Longitude\": \"12.500000000\", \"Latitude\": \"46.800000000\"},{\"Longitude\": \"13.500000000\", \"Latitude\": \"46.800000000\"},{\"Longitude\": \"13.500000000\", \"Latitude\": \"45.800000000\"},{\"Longitude\": \"12.500000000\", \"Latitude\": \"45.800000000\"}]}");

    myRealityData->SetFootprintString("{\"Coordinates\": [{\"Longitude\": \"0.00000000\", \"Latitude\": \"0.00000000\"},{\"Longitude\": \"1.000000000\", \"Latitude\": \"0.000000000\"},{\"Longitude\": \"1.000000000\", \"Latitude\": \"1.00000000\"},{\"Longitude\": \"0.00000000\", \"Latitude\": \"1.000000000\"},{\"Longitude\": \"0.000000000\", \"Latitude\": \"0.00000000\"}]}");
    EXPECT_STREQ(myRealityData->GetFootprintString().c_str(), "{\"Coordinates\": [{\"Longitude\": \"0.00000000\", \"Latitude\": \"0.00000000\"},{\"Longitude\": \"1.000000000\", \"Latitude\": \"0.000000000\"},{\"Longitude\": \"1.000000000\", \"Latitude\": \"1.00000000\"},{\"Longitude\": \"0.00000000\", \"Latitude\": \"1.000000000\"},{\"Longitude\": \"0.000000000\", \"Latitude\": \"0.00000000\"}]}");



    myRealityData->SetOrganizationId("2f1f7680-1be0-4e3f-9df4-cd7e72efcbcf");
    EXPECT_STREQ(myRealityData->GetOrganizationId().c_str(), "2f1f7680-1be0-4e3f-9df4-cd7e72efcbcf");
    myRealityData->SetContainerName("167b96ea-52eb-46c0-9865-8b7e5913bb29");
    EXPECT_STREQ(myRealityData->GetContainerName().c_str(), "167b96ea-52eb-46c0-9865-8b7e5913bb29");
    myRealityData->SetDataLocationGuid("99999999-9999-9999-9999-999999999999");
    EXPECT_STREQ(myRealityData->GetDataLocationGuid().c_str(), "99999999-9999-9999-9999-999999999999");
    myRealityData->SetDescription("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt");
    EXPECT_STREQ(myRealityData->GetDescription().c_str(), "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt");
    myRealityData->SetRootDocument("./RootDocument");
    EXPECT_STREQ(myRealityData->GetRootDocument().c_str(), "./RootDocument");
    myRealityData->SetAccuracy("2.34");
    EXPECT_STREQ(myRealityData->GetAccuracy().c_str(), "2.34");
    myRealityData->SetVisibilityByTag("ENTERPRISE");
    EXPECT_STREQ(myRealityData->GetVisibilityTag().c_str(), "ENTERPRISE"); // Default is preset
    myRealityData->SetListable(false);
    EXPECT_TRUE(!myRealityData->IsListable());
    myRealityData->SetOwner("Francis.Boily@Bentley.com;Alain.Robert@Bentley.com;PROJECT:af8c72c7-535b-4068-aebb-12d5fa9c688b");
    EXPECT_STREQ(myRealityData->GetOwner().c_str(), "Francis.Boily@Bentley.com;Alain.Robert@Bentley.com;PROJECT:af8c72c7-535b-4068-aebb-12d5fa9c688b");

    myRealityData->SetCreatorId("6e4f68b1-fe63-4264-a7de-f6d54abeeaef");
    EXPECT_STREQ(myRealityData->GetCreatorId().c_str(), "6e4f68b1-fe63-4264-a7de-f6d54abeeaef");

    myRealityData->SetMetadataUrl("http:\\www.bidon.com\\AgoodURL.html");
    EXPECT_STREQ(myRealityData->GetMetadataUrl().c_str(), "http:\\www.bidon.com\\AgoodURL.html");

    myRealityData->SetUltimateId("uId");
    EXPECT_STREQ(myRealityData->GetUltimateId().c_str(), "uId");

    myRealityData->SetUltimateSite("http:\\www.bidon.com\\AgoodURL.html");
    EXPECT_STREQ(myRealityData->GetUltimateSite().c_str(), "http:\\www.bidon.com\\AgoodURL.html");

    myRealityData->SetCopyright("Owned by Pinocchio");
    EXPECT_STREQ(myRealityData->GetCopyright().c_str(), "Owned by Pinocchio");

    myRealityData->SetTermsOfUse("Use with permisison of Tinkerbell");
    EXPECT_STREQ(myRealityData->GetTermsOfUse().c_str(), "Use with permisison of Tinkerbell");

    myRealityData->SetModifiedDateTime(DateTime(2017, 02, 27));
    EXPECT_TRUE(myRealityData->GetModifiedDateTime().IsValid());
    EXPECT_TRUE(myRealityData->GetModifiedDateTime() == DateTime(2017, 02, 27));

    myRealityData->SetLastAccessedDateTime(DateTime(2017, 02, 27));
    EXPECT_TRUE(myRealityData->GetLastAccessedDateTime().IsValid());
    EXPECT_TRUE(myRealityData->GetLastAccessedDateTime() == DateTime(2017, 02, 27));

    myRealityData->SetGroup("MyGroup1");
    EXPECT_STREQ(myRealityData->GetGroup().c_str(), "MyGroup1");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataCustomCreate)
    {
    bvector<GeoPoint2d> myFootprint;
    myFootprint.push_back(GeoPoint2d::From(12.5, 45.8));
    myFootprint.push_back(GeoPoint2d::From(12.5, 46.8));
    myFootprint.push_back(GeoPoint2d::From(13.5, 46.8));
    myFootprint.push_back(GeoPoint2d::From(13.5, 45.8));
    myFootprint.push_back(GeoPoint2d::From(12.5, 45.8));

    RealityDataPtr myRealityData = RealityData::Create("Identifier", DateTime(2017, 05, 8), "13.4x15.4", myFootprint, "MyName", "4578");
    EXPECT_STREQ(myRealityData->GetIdentifier().c_str(), "Identifier");
    EXPECT_TRUE(myRealityData->GetCreationDateTime() == DateTime(2017, 05, 8));
    EXPECT_STREQ(myRealityData->GetResolution().c_str(), "13.4x15.4");
    EXPECT_NEAR(myRealityData->GetResolutionValue(), 14.36, 0.01);

    DRange2d myRange = myRealityData->GetFootprintExtent();
    EXPECT_TRUE(myRange.low.x == 12.5);
    EXPECT_TRUE(myRange.high.x == 13.5);
    EXPECT_TRUE(myRange.low.y == 45.8);
    EXPECT_TRUE(myRange.high.y == 46.8);

    EXPECT_STREQ(myRealityData->GetFootprintString().c_str(), "{\"Coordinates\": [{\"Longitude\": \"12.500000000\", \"Latitude\": \"45.800000000\"},{\"Longitude\": \"12.500000000\", \"Latitude\": \"46.800000000\"},{\"Longitude\": \"13.500000000\", \"Latitude\": \"46.800000000\"},{\"Longitude\": \"13.500000000\", \"Latitude\": \"45.800000000\"},{\"Longitude\": \"12.500000000\", \"Latitude\": \"45.800000000\"}]}");

    EXPECT_STREQ(myRealityData->GetName().c_str(), "MyName");

    // Coordinate system is unused and we can't test it's value after being set in the constructor.
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataRelationshipBasicTest)
    {
    Utf8CP jsonString = "{"
                          "\"instances\": [" 
                            "{"
                            "\"instanceId\": \"14812\","
                            "\"schemaName\" : \"RealityModeling\","
                            "\"className\" : \"RealityDataRelationship\","
                            "\"properties\" : {"
                                  "\"RealityDataId\": \"f4425509-55c4-4e03-932a-d67b87ace30f\","
                                  "\"RelatedId\" : \"504fc784-2b2d-465f-b1d9-de58bf6cf0f2\","
                                  "\"RelationType\" : \"CONNECT-Project\","
                                  "\"ModifiedTimestamp\": \"0001-01-01T00:00:00.0000000\","
                                  "\"CreatedTimestamp\": \"0001-01-01T00:00:00.0000000\""
                               " },"
                            " \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\""
                           "},"
                            "{"
                            "\"instanceId\": \"14813\","
                            "\"schemaName\" : \"RealityModeling\","
                            "\"className\" : \"RealityDataRelationship\","
                            "\"properties\" : {"
                                  "\"RealityDataId\": \"8411d048-78ec-495a-b263-cad44dba7a17\","
                                  "\"RelatedId\" : \"73597d7f-e2fe-4704-8ee9-be37ed1f3d37\","
                                  "\"RelationType\" : \"CONNECT-Project\","
                                  "\"ModifiedTimestamp\": \"2018-01-23T12:45:23.4560000Z\","
                                  "\"CreatedTimestamp\": \"2017-11-14T00:33:20.0560000Z\""
                               " },"
                            " \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\""
                           "}"
                       " ]"
                     " }";
    // Parse.
    Json::Value root(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonString, root));

    // Instances must be a root node.
    ASSERT_TRUE(root.isMember("instances"));

    // Loop through all data and get required informations.
    const Json::Value instance = root["instances"][0];
    ASSERT_TRUE(instance.isMember("properties"));

    RealityDataRelationshipPtr myRelationShip = RealityDataRelationship::Create(instance);
    EXPECT_STREQ(myRelationShip->GetRealityDataId().c_str(), "f4425509-55c4-4e03-932a-d67b87ace30f");
    EXPECT_STREQ(myRelationShip->GetRelatedId().c_str(), "504fc784-2b2d-465f-b1d9-de58bf6cf0f2");
    EXPECT_STREQ(myRelationShip->GetRelationType().c_str(), "CONNECT-Project");

    EXPECT_TRUE(myRelationShip->GetModifiedDateTime().IsValid());
    EXPECT_TRUE(myRelationShip->GetModifiedDateTime() == DateTime(DateTime::Kind::Unspecified, 0001, 01, 01, 00, 00, 00, 0));
    EXPECT_TRUE(myRelationShip->GetCreationDateTime().IsValid());
    EXPECT_TRUE(myRelationShip->GetCreationDateTime() == DateTime(DateTime::Kind::Unspecified, 0001, 01, 01, 00, 00, 00, 0));

    const Json::Value instance2 = root["instances"][1];
    ASSERT_TRUE(instance.isMember("properties"));
    RealityDataRelationshipPtr myRelationShip2 = RealityDataRelationship::Create(instance2);
    EXPECT_STREQ(myRelationShip2->GetRealityDataId().c_str(), "8411d048-78ec-495a-b263-cad44dba7a17");
    EXPECT_STREQ(myRelationShip2->GetRelatedId().c_str(), "73597d7f-e2fe-4704-8ee9-be37ed1f3d37");
    EXPECT_STREQ(myRelationShip2->GetRelationType().c_str(), "CONNECT-Project");
    EXPECT_TRUE(myRelationShip2->GetModifiedDateTime().IsValid());
    EXPECT_TRUE(myRelationShip2->GetModifiedDateTime() == DateTime(DateTime::Kind::Utc, 2018,01,23, 12, 45, 23, 456));
    EXPECT_TRUE(myRelationShip2->GetCreationDateTime().IsValid());
    EXPECT_TRUE(myRelationShip2->GetCreationDateTime() == DateTime(DateTime::Kind::Utc, 2017,11,14, 0, 33, 20, 56));
    //empty
    RealityDataRelationshipPtr myRelationShip3 = RealityDataRelationship::Create();
    EXPECT_TRUE(myRelationShip3->GetRealityDataId().empty());
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataRelationshipGetSet)
    {
    RealityDataRelationshipPtr myRelationShip = RealityDataRelationship::Create();
    ASSERT_STREQ(myRelationShip->GetRelatedId().c_str(), "");
    ASSERT_STREQ(myRelationShip->GetRealityDataId().c_str(), "");
    ASSERT_STREQ(myRelationShip->GetRelationType().c_str(), "");

    myRelationShip->SetRelatedId("73597d7f-e2fe-4704-8ee9-be37ed1f3d37");
    ASSERT_STREQ(myRelationShip->GetRelatedId().c_str(), "73597d7f-e2fe-4704-8ee9-be37ed1f3d37");

    myRelationShip->SetRealityDataId("8411d048-78ec-495a-b263-cad44dba7a17");
    ASSERT_STREQ(myRelationShip->GetRealityDataId().c_str(), "8411d048-78ec-495a-b263-cad44dba7a17");

    myRelationShip->SetRelationType("CONNECT-Project");
    ASSERT_STREQ(myRelationShip->GetRelationType().c_str(), "CONNECT-Project");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataDocumentBasicTest)
    {
    RealityDataDocumentPtr emptyDocument = RealityDataDocument::Create();

    EXPECT_TRUE(emptyDocument != nullptr);

    Utf8CP jsonString = "{"
        "\"instances\": ["
        "{"
        "\"instanceId\": \"14812\","
        "\"schemaName\" : \"RealityModeling\","
        "\"className\" : \"RealityDataDocument\","
        "\"properties\" : {"
        "\"ContainerName\": \"f4425509-55c4-4e03-932a-d67b87ace30f\","
        "\"Name\" : \"Production_Helsinki_3MX_ok.3mx\","
        "\"Id\" : \"43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene/Production_Helsinki_3MX_ok.3mx\","
        "\"FolderId\" : \"43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene/\","
        "\"AccessUrl\" : \"https://dev-realitydataservices-eus.cloudapp.net/v2.4/Repositories/S3MXECPlugin--Server/S3MX/Document/43a4a51a-bfd3-4271-a9d9-21db56cdcf10~2FScene~2FProduction_Helsinki_3MX_ok.3mx/$file\","
        "\"RealityDataId\" : \"f4425509-55c4-4e03-932a-d67b87ace30f\","
        "\"ContentType\" : \"3mx\","
        "\"Size\" : \"1399\""
        " },"
        " \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\""
        "},"
        "{"
        "\"instanceId\": \"14813\","
        "\"schemaName\" : \"RealityModeling\","
        "\"className\" : \"RealityDataDocument\","
        "\"properties\" : {"
        "\"ContainerName\": \"f4425509-55c4-4e03-932a-d67b87ace30f\","
        "\"DataLocationGuid\": \"99999999-9999-9999-9999-999999999999\","
        "\"Name\" : \"Marseille.3mx\","
        "\"Id\" : \"TBD\","
        "\"FolderId\" : \"\","
        "\"AccessUrl\" : \"?????\","
        "\"RealityDataId\" : \"f4425509-55c4-4e03-932a-d67b87ace30f\","
        "\"ContentType\" : \"3mx\","
        "\"Size\" : \"\""
        " },"
        " \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\""
        "}"
        " ]"
        " }";
    // Parse.
    Json::Value root(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonString, root));

    // Instances must be a root node.
    ASSERT_TRUE(root.isMember("instances"));

    // Loop through all data and get required informations.
    const Json::Value instance = root["instances"][0];
    ASSERT_TRUE(instance.isMember("properties"));

    RealityDataDocumentPtr myDocument = RealityDataDocument::Create(instance);

    EXPECT_STREQ(myDocument->GetContainerName().c_str(), "f4425509-55c4-4e03-932a-d67b87ace30f");
    EXPECT_STREQ(myDocument->GetName().c_str(), "Production_Helsinki_3MX_ok.3mx");
    EXPECT_STREQ(myDocument->GetFolderId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene/");
    EXPECT_STREQ(myDocument->GetAccessUrl().c_str(), "https://dev-realitydataservices-eus.cloudapp.net/v2.4/Repositories/S3MXECPlugin--Server/S3MX/Document/43a4a51a-bfd3-4271-a9d9-21db56cdcf10~2FScene~2FProduction_Helsinki_3MX_ok.3mx/$file");
    EXPECT_STREQ(myDocument->GetRealityDataId().c_str(), "f4425509-55c4-4e03-932a-d67b87ace30f");
    EXPECT_STREQ(myDocument->GetId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene/Production_Helsinki_3MX_ok.3mx");
    EXPECT_STREQ(myDocument->GetContentType().c_str(), "3mx");
    EXPECT_TRUE(myDocument->GetSize() == 1399);

    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataDocumentSetStringId)
    {
    RealityDataDocumentPtr myDocument = RealityDataDocument::Create();
    myDocument->SetId("MyID");
    EXPECT_STREQ(myDocument->GetId().c_str(), "MyID");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataDocumentSetIdFromFolderName)
    {
    RealityDataDocumentPtr myDocument = RealityDataDocument::Create();
    myDocument->SetId("43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene", "Production_Helsinki_3MX_ok.3mx");
    EXPECT_STREQ(myDocument->GetId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene~2FProduction_Helsinki_3MX_ok.3mx");
    EXPECT_STREQ(myDocument->GetFolderId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene");
    EXPECT_STREQ(myDocument->GetName().c_str(), "Production_Helsinki_3MX_ok.3mx");
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataDocumentSetRealityDataId)
    {
    RealityDataDocumentPtr myDocument = RealityDataDocument::Create();
    myDocument->SetRealityDataId("MyRealityID");
    EXPECT_STREQ(myDocument->GetRealityDataId().c_str(), "MyRealityID");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataDocumentSetIdFromDataFolder)
    {
    Utf8CP jsonString = "{"
        "\"instances\": ["
        "{"
        "\"instanceId\": \"14812\","
        "\"schemaName\" : \"RealityModeling\","
        "\"className\" : \"RealityDataDocument\","
        "\"properties\" : {"
        "\"ContainerName\": \"f4425509-55c4-4e03-932a-d67b87ace30f\","
        "\"Name\" : \"Production_Helsinki_3MX_ok.3mx\","
        "\"Id\" : \"43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene\","
        "\"FolderId\" : \"43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene/\","
        "\"AccessUrl\" : \"https://dev-realitydataservices-eus.cloudapp.net/v2.4/Repositories/S3MXECPlugin--Server/S3MX/Document/43a4a51a-bfd3-4271-a9d9-21db56cdcf10~2FScene~2FProduction_Helsinki_3MX_ok.3mx/$file\","
        "\"RealityDataId\" : \"f4425509-55c4-4e03-932a-d67b87ace30f\","
        "\"ContentType\" : \"3mx\","
        "\"ParentFolderId\" : \"parentsIDDdD\","
        "\"Size\" : \"1399\""
        " },"
        " \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\""
        "},"
        "{"
        "\"instanceId\": \"14813\","
        "\"schemaName\" : \"RealityModeling\","
        "\"className\" : \"RealityDataDocument\","
        "\"properties\" : {"
        "\"ContainerName\": \"f4425509-55c4-4e03-932a-d67b87ace30f\","
        "\"Name\" : \"Marseille.3mx\","
        "\"Id\" : \"TBD\","
        "\"FolderId\" : \"\","
        "\"AccessUrl\" : \"?????\","
        "\"RealityDataId\" : \"f4425509-55c4-4e03-932a-d67b87ace30f\","
        "\"ContentType\" : \"3mx\","
        "\"ParentFolderId\" : \"parentsIDDdD\","
        "\"Size\" : \"\""
        " },"
        " \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\""
        "}"
        " ]"
        " }";

    // Parse.
    Json::Value root(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonString, root));
    const Json::Value instance = root["instances"][0];

    RealityDataFolderPtr dataFolderPtr = RealityDataFolder::Create(instance);
    RealityDataFolderCR datafolder = *dataFolderPtr;
    RealityDataDocumentPtr myDocument = RealityDataDocument::Create();
    myDocument->SetId(datafolder, "Production_Helsinki_3MX_ok.3mx");
    EXPECT_STREQ(myDocument->GetId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene~2FProduction_Helsinki_3MX_ok.3mx");
    EXPECT_STREQ(myDocument->GetFolderId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56cdcf10/Scene");
    EXPECT_STREQ(myDocument->GetName().c_str(), "Production_Helsinki_3MX_ok.3mx");
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataFolderBasicTest)
    {
    RealityDataFolderPtr myFolder = RealityDataFolder::Create(s_SampleRealityDataFolderJSON);

    EXPECT_STREQ(myFolder->GetName().c_str(), "Scene");
    EXPECT_STREQ(myFolder->GetParentId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56cdcf10/");
    EXPECT_STREQ(myFolder->GetRealityDataId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56cdcf10");

    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataFolderSetIdFromString)
    {
    RealityDataFolderPtr myFolder = RealityDataFolder::Create(s_SampleRealityDataFolderJSON);

    myFolder->SetId("43a4a51a-bfd3-4271-a9d9-21db56becf10");
    EXPECT_STREQ(myFolder->GetId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56becf10");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataFolderSetIdFromFolderIdAndName)
    {
    RealityDataFolderPtr myFolder = RealityDataFolder::Create(s_SampleRealityDataFolderJSON);

    myFolder->SetId("2523b4ee-4641-46a6-82a8-f26134f3b099", "43a4a51a-bfd3-4271-a9d9-21db56becf10");
    EXPECT_STREQ(myFolder->GetId().c_str(), "2523b4ee-4641-46a6-82a8-f26134f3b099~2F43a4a51a-bfd3-4271-a9d9-21db56becf10");
    EXPECT_STREQ(myFolder->GetName().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56becf10");
    EXPECT_STREQ(myFolder->GetParentId().c_str(), "2523b4ee-4641-46a6-82a8-f26134f3b099");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataFolderSetIdFromRealityDataFolderAndName)
    {
    RealityDataFolderPtr seedFolder = RealityDataFolder::Create(s_SampleRealityDataFolderJSON);
    seedFolder->SetId("6deb80b6-ec28-410e-8857-a33f4d0f74df");
    RealityDataFolderPtr folderUnderTest = RealityDataFolder::Create(s_SampleRealityDataFolderJSON);

    folderUnderTest->SetId(*seedFolder, "43a4a51a-bfd3-4271-a9d9-21db56becf10");
    EXPECT_STREQ(folderUnderTest->GetId().c_str(), "6deb80b6-ec28-410e-8857-a33f4d0f74df~2F43a4a51a-bfd3-4271-a9d9-21db56becf10");
    EXPECT_STREQ(folderUnderTest->GetName().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56becf10");
    EXPECT_STREQ(folderUnderTest->GetParentId().c_str(), "6deb80b6-ec28-410e-8857-a33f4d0f74df");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataFolderSetRealityDataId)
    {
    RealityDataFolderPtr folderUnderTest = RealityDataFolder::Create(s_SampleRealityDataFolderJSON);

    folderUnderTest->SetRealityDataId("43a4a51a-bfd3-4271-a9d9-21db56becf10");
    EXPECT_STREQ(folderUnderTest->GetRealityDataId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56becf10");
    }


//-------------------------------------------------------------------------------------
// @bsiclass                         Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
struct classificationTag_state
    {
    RealityDataBase::Classification classificationTag;
    Utf8CP classificationName;
    StatusInt status;

    friend std::ostream& operator<<(std::ostream& os, const classificationTag_state& obj)
        {
        return os
            << "Tag: " << obj.classificationTag
            << " name: " << obj.classificationName
            << " status: " << obj.status;
        }
    };

//-------------------------------------------------------------------------------------
// @bsiclass                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
struct RealityDataBaseClassification : testing::Test, ::testing::WithParamInterface<classificationTag_state>
    {

    };

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_P(RealityDataBaseClassification, ConversionNameToEnum)
    {
    RealityDataBase::Classification returnedClassification(RealityDataBase::Classification::MODEL);
    StatusInt status = RealityDataBase::GetClassificationFromTag(returnedClassification, GetParam().classificationName);

    EXPECT_TRUE(status == GetParam().status);
    EXPECT_TRUE(returnedClassification == GetParam().classificationTag);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_P(RealityDataBaseClassification, ConversionEnumToName)
    {
    auto returnedClassification = RealityDataBase::GetTagFromClassification(GetParam().classificationTag);

    EXPECT_STREQ(returnedClassification.c_str(), GetParam().classificationName);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataBaseClassification, BadConversion)
    {
    RealityDataBase::Classification returnedClassification(RealityDataBase::Classification::MODEL);
    StatusInt status = RealityDataBase::GetClassificationFromTag(returnedClassification, "NotARealTag");

    EXPECT_TRUE(status == ERROR);
    }

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
INSTANTIATE_TEST_CASE_P(Default, RealityDataBaseClassification, testing::Values(
    classificationTag_state {RealityDataBase::Classification::MODEL, "Model", SUCCESS},
    classificationTag_state {RealityDataBase::Classification::TERRAIN, "Terrain", SUCCESS},
    classificationTag_state {RealityDataBase::Classification::IMAGERY, "Imagery", SUCCESS},
    classificationTag_state {RealityDataBase::Classification::PINNED, "Pinned", SUCCESS},
    classificationTag_state {RealityDataBase::Classification::UNDEFINED_CLASSIF, "Undefined", SUCCESS}
));

struct visibilityTag_state
    {
    RealityDataBase::Visibility visibilityTag;
    Utf8CP visibilityName;
    StatusInt status;

    friend std::ostream& operator<<(std::ostream& os, const visibilityTag_state& obj)
        {
        return os
            << "Tag: " << obj.visibilityTag
            << " name: " << obj.visibilityName
            << " status: " << obj.status;
        }
    };

//-------------------------------------------------------------------------------------
// @bsiclass                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
struct RealityDataBaseVisibility : testing::Test, ::testing::WithParamInterface<visibilityTag_state>
    {

    };

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_P(RealityDataBaseVisibility, ConversionNameToEnum)
    {
    RealityDataBase::Visibility returnedVisibility(RealityDataBase::Visibility::ENTERPRISE);
    StatusInt status = RealityDataBase::GetVisibilityFromTag(returnedVisibility, GetParam().visibilityName);

    EXPECT_TRUE(status == GetParam().status);
    EXPECT_TRUE(returnedVisibility == GetParam().visibilityTag);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_P(RealityDataBaseVisibility, ConversionEnumToName)
    {
    auto returnedClassification = RealityDataBase::GetTagFromVisibility(GetParam().visibilityTag);

    EXPECT_STREQ(returnedClassification.c_str(), GetParam().visibilityName);
    }

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
INSTANTIATE_TEST_CASE_P(Default, RealityDataBaseVisibility, testing::Values(
    visibilityTag_state {RealityDataBase::Visibility::PRIVATE, "PRIVATE", SUCCESS},
    visibilityTag_state {RealityDataBase::Visibility::PERMISSION, "PERMISSION", SUCCESS},
    visibilityTag_state {RealityDataBase::Visibility::ENTERPRISE, "ENTERPRISE", SUCCESS},
    visibilityTag_state {RealityDataBase::Visibility::UNDEFINED_VISIBILITY, "UNDEFINED", SUCCESS}
));

//-------------------------------------------------------------------------------------
// @bsimethod                          Remi.Charbonneau                         05/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, SpatialEntityThumbnailBase)
    {
    auto thumbnailUnderTest = SpatialEntityThumbnail::Create();

    EXPECT_TRUE(thumbnailUnderTest->IsEmpty());

    thumbnailUnderTest->SetProvenance("MyProvenance");
    EXPECT_FALSE(thumbnailUnderTest->IsEmpty());
    EXPECT_STREQ(thumbnailUnderTest->GetProvenance().c_str(), "MyProvenance");

    thumbnailUnderTest->SetFormat("superJPEG");
    EXPECT_STREQ(thumbnailUnderTest->GetFormat().c_str(), "superJPEG");

    thumbnailUnderTest->SetWidth(555);
    EXPECT_EQ(thumbnailUnderTest->GetWidth(), 555);

    thumbnailUnderTest->SetHeight(666);
    EXPECT_EQ(thumbnailUnderTest->GetHeight(), 666);

    auto inputData = bvector<Byte>({static_cast<Byte>(1), static_cast<Byte>(1), static_cast<Byte>(2), static_cast<Byte>(3)});
    thumbnailUnderTest->SetData(inputData);
    auto outputData = thumbnailUnderTest->GetData();
    EXPECT_TRUE(std::equal(inputData.begin(), inputData.end(), outputData.begin()));

    thumbnailUnderTest->SetGenerationDetails("MyGenerationDetails");
    EXPECT_STREQ(thumbnailUnderTest->GetGenerationDetails().c_str(), "MyGenerationDetails");
    }