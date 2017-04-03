//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/RealityDataObjectsTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST


#include <Bentley/BeTest.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/SpatialEntity.h>
#include <RealityPlatform/RealityDataObjects.h>
#include <RealityPlatform/RealityConversionTools.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
// @bsimethod                          Alain.Robert                            02/2017
//=====================================================================================
class RealityDataObjectTestFixture : public testing::Test
    {
    public:
    
    };

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


    EXPECT_TRUE(mySpatialEntity->GetDataSourceCount() == 0);

    mySpatialEntity->SetDescription("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt"); 
    EXPECT_STREQ(mySpatialEntity->GetDescription().c_str(), "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt");
    mySpatialEntity->SetAccuracy("2.34"); 
    EXPECT_STREQ(mySpatialEntity->GetAccuracy().c_str(), "2.34");
    mySpatialEntity->SetVisibilityByTag("PUBLIC"); 
    EXPECT_STREQ(mySpatialEntity->GetVisibilityTag().c_str(), "PUBLIC"); // Default is preset

    EXPECT_TRUE(mySpatialEntity->GetMetadataCP() == NULL);

    mySpatialEntity->SetOcclusion(23.45); 
    EXPECT_NEAR(mySpatialEntity->GetOcclusion(), 23.45, 0.00001);
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

    EXPECT_STREQ(mySpatialEntityMetadata->GetProvenance().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetLineage().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetDescription().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetContactInfo().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetLegal().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetTermsOfUse().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetKeywords().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetFormat().c_str(), "");
    EXPECT_STREQ(mySpatialEntityMetadata->GetMetadataUrl().c_str(), "");

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

    EXPECT_STREQ(mySpatialEntityDataSource->GetUrl().c_str(), "");

    EXPECT_STREQ(mySpatialEntityDataSource->GetGeoCS().c_str(), "");

    EXPECT_STREQ(mySpatialEntityDataSource->GetCompoundType().c_str(), "");

    EXPECT_TRUE(mySpatialEntityDataSource->GetSize() == 0);

    EXPECT_STREQ(mySpatialEntityDataSource->GetNoDataValue().c_str(), "");

    EXPECT_STREQ(mySpatialEntityDataSource->GetDataType().c_str(), "");

    EXPECT_STREQ(mySpatialEntityDataSource->GetLocationInCompound().c_str(), "");

    ASSERT_TRUE(!mySpatialEntityDataSource->GetIsMultiband());


    Utf8String redUrl;
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

    ASSERT_TRUE(mySpatialEntityDataSource->GetPanchromaticBandSize() == 0);

    // Set test
    mySpatialEntityDataSource->SetUrl("http:\\\\somewhere.com\\SomeLocation");
    EXPECT_STREQ(mySpatialEntityDataSource->GetUrl().c_str(), "http:\\\\somewhere.com\\SomeLocation");

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

    mySpatialEntityDataSource->SetIsMultiband(true);
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
    ASSERT_TRUE(mySpatialEntityDataSource->GetPanchromaticBandSize() == 2345);
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
    EXPECT_STREQ(mySpatialEntityServer->GetState() .c_str(), "");
    EXPECT_TRUE(!mySpatialEntityServer->IsStreamed());
    EXPECT_STREQ(mySpatialEntityServer->GetLoginKey().c_str(), "");
    EXPECT_STREQ(mySpatialEntityServer->GetLoginMethod().c_str(), "");

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

    mySpatialEntityServer->SetOnline(true);
    EXPECT_TRUE(mySpatialEntityServer->IsOnline());

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
    mySpatialEntity->SetVisibilityByTag("PUBLIC"); 

    mySpatialEntity->SetOcclusion(23.45); 





    // Create and add a spatial data source
    SpatialEntityDataSourcePtr mySpatialEntityDataSource = SpatialEntityDataSource::Create();

    mySpatialEntityDataSource->SetUrl("http:\\\\somewhere.com\\SomeLocation");
    mySpatialEntityDataSource->SetGeoCS("EPSG:4326;NAVD88");
    mySpatialEntityDataSource->SetCompoundType("zip");
    mySpatialEntityDataSource->SetSize(12345);
    mySpatialEntityDataSource->SetNoDataValue("0:0:0");
    mySpatialEntityDataSource->SetDataType("tif");
    mySpatialEntityDataSource->SetLocationInCompound(".\\a.tif");
    mySpatialEntityDataSource->SetIsMultiband(true);
    mySpatialEntityDataSource->SetMultibandUrls("red.tif", "green.tif", "blue.tif", "pan.tif");
    mySpatialEntityDataSource->SetRedBandSize(324);
    mySpatialEntityDataSource->SetGreenBandSize(321);
    mySpatialEntityDataSource->SetBlueBandSize(453);
    mySpatialEntityDataSource->SetPanchromaticBandSize(2345);
    
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
    EXPECT_TRUE(myRealityData->GetStreamed() == true);
    EXPECT_TRUE(myRealityData->GetClassification() == RealityData::Classification::UNDEFINED_CLASSIF);
    ASSERT_STRCASEEQ(myRealityData->GetClassificationTag().c_str(), "Undefined"); // Default is preset
    EXPECT_STREQ(myRealityData->GetDataset().c_str(), "");
    EXPECT_STREQ(myRealityData->GetThumbnailDocument().c_str(), "");
    EXPECT_TRUE(myRealityData->GetTotalSize() == 0);
    EXPECT_TRUE(!myRealityData->GetCreationDateTime().IsValid()); // Time not set should be invalid
    EXPECT_TRUE(myRealityData->GetFootprint().size() == 0);
    EXPECT_TRUE(!myRealityData->HasApproximateFootprint());
    DRange2d myRange = myRealityData->GetFootprintExtent();
    EXPECT_TRUE(myRange.low.x == 0.0);
    EXPECT_TRUE(myRange.high.x == 0.0);
    EXPECT_TRUE(myRange.low.y == 0.0);
    EXPECT_TRUE(myRange.high.y == 0.0);


    EXPECT_STREQ(myRealityData->GetEnterpriseId().c_str(), "");
    EXPECT_STREQ(myRealityData->GetContainerName().c_str(), "");
    EXPECT_STREQ(myRealityData->GetDescription().c_str(), "");
    EXPECT_STREQ(myRealityData->GetRootDocument().c_str(), "");
    EXPECT_STREQ(myRealityData->GetAccuracy().c_str(), "");
    EXPECT_STREQ(myRealityData->GetVisibilityTag().c_str(), "UNDEFINED"); // Default is preset
    EXPECT_TRUE(myRealityData->IsListable());
    EXPECT_STREQ(myRealityData->GetOwner().c_str(), "");

    EXPECT_STREQ(myRealityData->GetMetadataURL().c_str(), "");
    EXPECT_STREQ(myRealityData->GetCopyright().c_str(), "");
    EXPECT_STREQ(myRealityData->GetTersmOfUse().c_str(), "");
    EXPECT_TRUE(!myRealityData->GetModifiedDateTime().IsValid()); // Time not set should be invalid
    EXPECT_STREQ(myRealityData->GetGroup().c_str(), "");

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
    ASSERT_TRUE(myRealityData->GetStreamed() == false);
    myRealityData->SetClassification(RealityData::Classification::MODEL); 
    ASSERT_TRUE(myRealityData->GetClassification() == RealityData::Classification::MODEL);
    ASSERT_STRCASEEQ(myRealityData->GetClassificationTag().c_str(), "Model");
    myRealityData->SetDataset("MyDataset1"); 
    EXPECT_STREQ(myRealityData->GetDataset().c_str(), "MyDataset1");
    myRealityData->SetThumbnailDocument("thumbnail.jpg"); 
    EXPECT_STREQ(myRealityData->GetThumbnailDocument().c_str(), "thumbnail.jpg");
    myRealityData->SetTotalSize(123765473); 
    EXPECT_TRUE(myRealityData->GetTotalSize() == 123765473);
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



    myRealityData->SetEnterpriseId("2f1f7680-1be0-4e3f-9df4-cd7e72efcbcf"); 
    EXPECT_STREQ(myRealityData->GetEnterpriseId().c_str(), "2f1f7680-1be0-4e3f-9df4-cd7e72efcbcf");
    myRealityData->SetContainerName("167b96ea-52eb-46c0-9865-8b7e5913bb29"); 
    EXPECT_STREQ(myRealityData->GetContainerName().c_str(), "167b96ea-52eb-46c0-9865-8b7e5913bb29");
    myRealityData->SetDescription("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt"); 
    EXPECT_STREQ(myRealityData->GetDescription().c_str(), "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt");
    myRealityData->SetRootDocument("./RootDocument"); 
    EXPECT_STREQ(myRealityData->GetRootDocument().c_str(), "./RootDocument");
    myRealityData->SetAccuracy("2.34"); 
    EXPECT_STREQ(myRealityData->GetAccuracy().c_str(), "2.34");
    myRealityData->SetVisibilityByTag("PUBLIC"); 
    EXPECT_STREQ(myRealityData->GetVisibilityTag().c_str(), "PUBLIC"); // Default is preset
    myRealityData->SetListable(false); 
    EXPECT_TRUE(!myRealityData->IsListable());
    myRealityData->SetOwner("Francis.Boily@Bentley.com;Alain.Robert@Bentley.com;PROJECT:af8c72c7-535b-4068-aebb-12d5fa9c688b"); 
    EXPECT_STREQ(myRealityData->GetOwner().c_str(), "Francis.Boily@Bentley.com;Alain.Robert@Bentley.com;PROJECT:af8c72c7-535b-4068-aebb-12d5fa9c688b");

    myRealityData->SetMetadataURL("http:\\www.bidon.com\AgoodURL.html"); 
    EXPECT_STREQ(myRealityData->GetMetadataURL().c_str(), "http:\\www.bidon.com\AgoodURL.html");


    myRealityData->SetCopyright("Owned by Pinocchio"); 
    EXPECT_STREQ(myRealityData->GetCopyright().c_str(), "Owned by Pinocchio");

    myRealityData->SetTermsOfUse("Use with permisison of Tinkerbell"); 
    EXPECT_STREQ(myRealityData->GetTermsOfUse().c_str(), "Use with permisison of Tinkerbell");

    // myRealityData->SetModifiedTimestamp(DateTime::GetCurrentTime()); 
    // EXPECT_TRUE(!myRealityData->GetModifiedTimestamp().IsValid()); // Time not set should be invalid

    myRealityData->SetGroup("MyGroup1"); 
    EXPECT_STREQ(myRealityData->GetGroup().c_str(), "MyGroup1");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataProjectRelationshipBasicTest)
    {
    Utf8CP jsonString = "{"
                          "\"instances\": [" 
                            "{"
                            "\"instanceId\": \"14812\","
                            "\"schemaName\" : \"RealityModeling\","
                            "\"className\" : \"RealityDataProjectRelationship\","
                            "\"properties\" : {"
                                  "\"RealityDataId\": \"f4425509-55c4-4e03-932a-d67b87ace30f\","
                                  "\"ProjectId\" : \"504fc784-2b2d-465f-b1d9-de58bf6cf0f2\""
                               " },"
                            " \"eTag\": \"gZIS2SFbXqohdwLlTRXkJOTCHz0=\""
                           "},"
                            "{"
                            "\"instanceId\": \"14813\","
                            "\"schemaName\" : \"RealityModeling\","
                            "\"className\" : \"RealityDataProjectRelationship\","
                            "\"properties\" : {"
                                  "\"RealityDataId\": \"8411d048-78ec-495a-b263-cad44dba7a17\","
                                  "\"ProjectId\" : \"73597d7f-e2fe-4704-8ee9-be37ed1f3d37\""
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

    RealityDataProjectRelationshipPtr myRelationShip = RealityDataProjectRelationship::Create(instance);
    EXPECT_STREQ(myRelationShip->GetRealityDataId().c_str(), "f4425509-55c4-4e03-932a-d67b87ace30f");
    EXPECT_STREQ(myRelationShip->GetProjectId().c_str(), "504fc784-2b2d-465f-b1d9-de58bf6cf0f2");

    const Json::Value instance2 = root["instances"][1];
    ASSERT_TRUE(instance.isMember("properties"));
    RealityDataProjectRelationshipPtr myRelationShip2 = RealityDataProjectRelationship::Create(instance2);
    EXPECT_STREQ(myRelationShip2->GetRealityDataId().c_str(), "8411d048-78ec-495a-b263-cad44dba7a17");
    EXPECT_STREQ(myRelationShip2->GetProjectId().c_str(), "73597d7f-e2fe-4704-8ee9-be37ed1f3d37");
    }




//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataDocumentBasicTest)
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
    EXPECT_STREQ(myDocument->GetContentType().c_str(), "3mx");
    EXPECT_TRUE(myDocument->GetSize() == 1399);

    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Alain.Robert                            02/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataObjectTestFixture, RealityDataFolderBasicTest)
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
    ASSERT_TRUE(Json::Reader::Parse(jsonString, root));

    // Instances must be a root node.
    ASSERT_TRUE(root.isMember("instances"));

    // Loop through all data and get required informations.
    const Json::Value instance = root["instances"][0];
    ASSERT_TRUE(instance.isMember("properties"));

    RealityDataFolderPtr myFolder = RealityDataFolder::Create(instance);

    EXPECT_STREQ(myFolder->GetName().c_str(), "Scene");
    EXPECT_STREQ(myFolder->GetParentId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56cdcf10/");
    EXPECT_STREQ(myFolder->GetRealityDataId().c_str(), "43a4a51a-bfd3-4271-a9d9-21db56cdcf10");

    }