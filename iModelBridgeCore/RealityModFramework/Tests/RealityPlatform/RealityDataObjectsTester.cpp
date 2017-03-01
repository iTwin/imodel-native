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
    EXPECT_TRUE(mySpatialEntity->GetClassification() == SpatialEntity::Classification::UNDEFINED);
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
    EXPECT_STREQ(mySpatialEntity->GetEnterprise().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetContainerName().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetDescription().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetRootDocument().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetAccuracy().c_str(), "");
    EXPECT_STREQ(mySpatialEntity->GetVisibilityTag().c_str(), "UNDEFINED"); // Default is preset
    EXPECT_TRUE(mySpatialEntity->IsListable());
    EXPECT_STREQ(mySpatialEntity->GetOwner().c_str(), "");

    EXPECT_TRUE(mySpatialEntity->GetMetadataCP() == NULL);
    EXPECT_TRUE(!mySpatialEntity->GetModifiedTimestamp().IsValid()); // Time not set should be invalid
    EXPECT_STREQ(mySpatialEntity->GetGroup().c_str(), "");
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

    mySpatialEntity->SetEnterprise("2f1f7680-1be0-4e3f-9df4-cd7e72efcbcf"); 
    EXPECT_STREQ(mySpatialEntity->GetEnterprise().c_str(), "2f1f7680-1be0-4e3f-9df4-cd7e72efcbcf");
    mySpatialEntity->SetContainerName("167b96ea-52eb-46c0-9865-8b7e5913bb29"); 
    EXPECT_STREQ(mySpatialEntity->GetContainerName().c_str(), "167b96ea-52eb-46c0-9865-8b7e5913bb29");
    mySpatialEntity->SetDescription("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt"); 
    EXPECT_STREQ(mySpatialEntity->GetDescription().c_str(), "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt");
    mySpatialEntity->SetRootDocument("./RootDocument"); 
    EXPECT_STREQ(mySpatialEntity->GetRootDocument().c_str(), "./RootDocument");
    mySpatialEntity->SetAccuracy("2.34"); 
    EXPECT_STREQ(mySpatialEntity->GetAccuracy().c_str(), "2.34");
    mySpatialEntity->SetVisibilityByTag("PUBLIC"); 
    EXPECT_STREQ(mySpatialEntity->GetVisibilityTag().c_str(), "PUBLIC"); // Default is preset
    mySpatialEntity->SetListable(false); 
    EXPECT_TRUE(!mySpatialEntity->IsListable());
    mySpatialEntity->SetOwner("Francis.Boily@Bentley.com;Alain.Robert@Bentley.com;PROJECT:af8c72c7-535b-4068-aebb-12d5fa9c688b"); 
    EXPECT_STREQ(mySpatialEntity->GetOwner().c_str(), "Francis.Boily@Bentley.com;Alain.Robert@Bentley.com;PROJECT:af8c72c7-535b-4068-aebb-12d5fa9c688b");

    EXPECT_TRUE(mySpatialEntity->GetMetadataCP() == NULL);

    // mySpatialEntity->SetModifiedTimestamp(DateTime::GetCurrentTime()); 
    // EXPECT_TRUE(!mySpatialEntity->GetModifiedTimestamp().IsValid()); // Time not set should be invalid

    mySpatialEntity->SetGroup("MyGroup1"); 
    EXPECT_STREQ(mySpatialEntity->GetGroup().c_str(), "MyGroup1");
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

    mySpatialEntity->SetEnterprise("2f1f7680-1be0-4e3f-9df4-cd7e72efcbcf"); 
    mySpatialEntity->SetContainerName("167b96ea-52eb-46c0-9865-8b7e5913bb29"); 
    mySpatialEntity->SetDescription("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque et est ac ligula pellentesque eleifend. Fusce congue quam tincidunt"); 
    mySpatialEntity->SetRootDocument("./RootDocument"); 
    mySpatialEntity->SetAccuracy("2.34"); 
    mySpatialEntity->SetVisibilityByTag("PUBLIC"); 
    mySpatialEntity->SetListable(false); 
    mySpatialEntity->SetOwner("Francis.Boily@Bentley.com;Alain.Robert@Bentley.com;PROJECT:af8c72c7-535b-4068-aebb-12d5fa9c688b"); 

    mySpatialEntity->SetModifiedTimestamp(DateTime(2017,02,17)); 
    mySpatialEntity->SetGroup("MyGroup1"); 
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