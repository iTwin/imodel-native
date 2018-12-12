//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityAdmin/ContextServicesTester.cpp $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST

#include <windows.h>
//#include <omp.h>

#include <Bentley/BeTest.h>
#include <Bentley/BeFile.h>
#include <RealityAdmin/ContextServicesWorkbench.h>
#include <RealityPlatformTools/RealityConversionTools.h>
#include <RealityPlatformTools/RealityDataDownload.h>
#include <RealityPlatformTools/WSGServices.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

#define BOX_ELLIPSE  0 
#define BOX_RECT     1 

#define CCH_MAXLABEL 80 
#define CX_MARGIN    12 

//=====================================================================================
//! @bsiclass                          Spencer.Mason                            10/2016
//=====================================================================================
class ContextServicesTestFixture : public testing::Test
    {
    public:
	Utf8String GetToken()
        {
        return ConnectTokenManager::GetInstance().GetToken();
        }


    BeFileName GetPemLocation()
        {
        WChar exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);

        WString exeDir = exePath;
        size_t pos = exeDir.find_last_of(L"/\\");
        exeDir = exeDir.substr(0, pos + 1);

        BeFileName caBundlePath(exeDir);
        caBundlePath.AppendToPath(L"Assets").AppendToPath(L"http").AppendToPath(L"cabundle.pem");
        return caBundlePath;
        }

    bvector<GeoPoint2d> GetDefaultPolygon()
        {
        bvector<GeoPoint2d> filterPolygon = bvector<GeoPoint2d>();
        GeoPoint2d p1, p2, p3, p4, p5;
        p1.Init(-79.25, 38.57);
        p2.Init(-79.05, 38.57);
        p3.Init(-79.05, 38.77);
        p4.Init(-79.25, 38.77);
        p5.Init(-79.25, 38.57);
        filterPolygon.push_back(p1);
        filterPolygon.push_back(p2);
        filterPolygon.push_back(p3);
        filterPolygon.push_back(p4);
        filterPolygon.push_back(p5);

        return filterPolygon;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  4/2015
    //----------------------------------------------------------------------------------------
    BeFileName GetXsdDirectory()
        {
        BeFileName outDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(outDir);
        outDir.AppendToPath(L"xsd");
        return outDir;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  4/2015
    //----------------------------------------------------------------------------------------
    BeFileName GetXsd_2_0()
        {
        BeFileName out = GetXsdDirectory();
        out.AppendToPath(L"RealityPackage.2.0.xsd");
        return out;
        }

    };

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Get the SpatialEntityWithDetailsView from a footprint
// Ensure a certain amount of results
// Ensure a certain amount of non-landsat results
// Test SpatioTemporal filtering
// Make sure no errors occur during package download
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, ConceptStationTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(GetDefaultPolygon());

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails());

    SpatialEntityDatasetPtr dataset = SpatialEntityDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i >= 458);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 9);

    cswBench->FilterSpatialEntity();

    RealityPlatform::SpatioTemporalSelector::ResolutionMap selectedIds = cswBench->GetSelectedIds();

    i = selectedIds.size();

    ASSERT_TRUE( i >= 3);

    bvector<Utf8String> low = selectedIds[ResolutionCriteria::Low];
    bvector<Utf8String> medium = selectedIds[ResolutionCriteria::Medium];
    bvector<Utf8String> high = selectedIds[ResolutionCriteria::High];

    i = low.size();
    ASSERT_TRUE(i >= 2);

    i = medium.size();
    ASSERT_TRUE(i >= 17);

    i = high.size();
    ASSERT_TRUE(i >= 216);

    cswBench->FilterSpatialEntity([](SpatialEntityPtr entity) -> bool { Json::Value provider = entity->GetProvider(); return provider.isString() ? provider.asString().EqualsI("Amazon Landsat 8") : false; });

    selectedIds = cswBench->GetSelectedIds();

    i = selectedIds.size();

    ASSERT_TRUE(i >= 3);

    low = selectedIds[ResolutionCriteria::Low];
    medium = selectedIds[ResolutionCriteria::Medium];
    high = selectedIds[ResolutionCriteria::High];

    i = low.size();
    ASSERT_TRUE(i >= 17);

    i = medium.size();
    ASSERT_TRUE(i >= 188);

    i = high.size();
    ASSERT_TRUE(i >= 216);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageId());
    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageFile());

    WCharCP packageFile = cswBench->GetPackageFileName().GetName();
    ASSERT_TRUE(BeFileName::DoesPathExist(packageFile));
    BeFileName::EmptyAndRemoveDirectory(packageFile);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Test the 'select' function; to return only certain fields
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, SelectTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(GetDefaultPolygon());

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&$select=Name,Footprint"));

    Json::Value json;
    Json::Reader::Parse(cswBench->GetSpatialEntityWithDetailsJson(), json);
    for(Json::Value instance : json["instances"])
        {
        ASSERT_TRUE(instance["properties"].size() == 2);
        ASSERT_TRUE(instance["properties"].isMember("Name"));
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Test the 'filter' function; to return only certain data types
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, FilterTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(GetDefaultPolygon());

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&$filter=Classification+eq+'Terrain'"));

    SpatialEntityDatasetPtr dataset = SpatialEntityDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i == 0);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 1);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Ensure a certain amount of results for an area in the USA
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, USTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(GetDefaultPolygon());

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails());

    SpatialEntityDatasetPtr dataset = SpatialEntityDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i >= 458);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 9);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Ensure a certain amount of results for an area in Australia
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, AustraliaTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    bvector<GeoPoint2d> filterPolygon = bvector<GeoPoint2d>();
    GeoPoint2d p1, p2, p3, p4, p5;
    p1.Init(130.25, -26.57);
    p2.Init(130.05, -26.57);
    p3.Init(130.05, -26.77);
    p4.Init(130.25, -26.77);
    p5.Init(130.25, -26.57);
    filterPolygon.push_back(p1);
    filterPolygon.push_back(p2);
    filterPolygon.push_back(p3);
    filterPolygon.push_back(p4);
    filterPolygon.push_back(p5);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(filterPolygon);

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails());

    SpatialEntityDatasetPtr dataset = SpatialEntityDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i >= 144);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 1);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Ensure a certain amount of results for an area in China
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, ChinaTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    bvector<GeoPoint2d> filterPolygon = bvector<GeoPoint2d>();
    GeoPoint2d p1, p2, p3, p4, p5;
    p1.Init(108.25, 30.57);
    p2.Init(108.05, 30.57);
    p3.Init(108.05, 30.77);
    p4.Init(108.25, 30.77);
    p5.Init(108.25, 30.57);
    filterPolygon.push_back(p1);
    filterPolygon.push_back(p2);
    filterPolygon.push_back(p3);
    filterPolygon.push_back(p4);
    filterPolygon.push_back(p5);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(filterPolygon);

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails());

    SpatialEntityDatasetPtr dataset = SpatialEntityDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i >= 178);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 1);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Test the source function to ensure that when you call the usgsapi, you only get 
// usgs results
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, SourceTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(GetDefaultPolygon());

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&source=usgsapi"));

    cswBench->FilterSpatialEntity();

    RealityPlatform::SpatioTemporalSelector::ResolutionMap selectedIds = cswBench->GetSelectedIds();

    bvector<Utf8String> low = selectedIds[ResolutionCriteria::Low];
    bvector<Utf8String> medium = selectedIds[ResolutionCriteria::Medium];
    bvector<Utf8String> high = selectedIds[ResolutionCriteria::High];

    size_t lowSize = low.size();
    size_t midSize = medium.size();
    size_t highSize = high.size();

    cswBench->FilterSpatialEntity([](SpatialEntityPtr entity) -> bool { Json::Value provider = entity->GetProvider(); return provider.isString() ? !provider.asString().ToUpper().EqualsI("USGS") : false; });

    selectedIds = cswBench->GetSelectedIds();

    low = selectedIds[ResolutionCriteria::Low];
    medium = selectedIds[ResolutionCriteria::Medium];
    high = selectedIds[ResolutionCriteria::High];

    ASSERT_TRUE(low.size() == lowSize);
    ASSERT_TRUE(medium.size() == midSize);
    ASSERT_TRUE(high.size() == highSize);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Ensure that the usgsapi is returning values
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, USGSTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(GetDefaultPolygon());

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&source=usgsapi"));

    SpatialEntityDatasetPtr dataset = SpatialEntityDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i >= 299);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 7);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageId());
    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageFile());

    WCharCP packageFile = cswBench->GetPackageFileName().GetName();
    ASSERT_TRUE(BeFileName::DoesPathExist(packageFile));
    BeFileName::EmptyAndRemoveDirectory(packageFile);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Ensure that the index api is returning values
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, IndexTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(GetDefaultPolygon());

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&source=index"));

    SpatialEntityDatasetPtr dataset = SpatialEntityDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i >= 159);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 2);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageId());
    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageFile());

    WCharCP packageFile = cswBench->GetPackageFileName().GetName();
    ASSERT_TRUE(BeFileName::DoesPathExist(packageFile));
    BeFileName::EmptyAndRemoveDirectory(packageFile);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Ensure that PackageRequest is returning a valid file name
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, PackageIdTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(GetDefaultPolygon());

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&$filter=Classification+eq+'Terrain'")); 
    //request only terrain to limit the amount of responses

    cswBench->FilterSpatialEntity();

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageId());
    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageFile());

    ASSERT_TRUE(cswBench->GetInstanceId().Contains(".xrdp"));

    WCharCP packageFile = cswBench->GetPackageFileName().GetName();
    ASSERT_TRUE(BeFileName::DoesPathExist(packageFile));
    BeFileName::EmptyAndRemoveDirectory(packageFile);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// Ensure that the package file is on the file system, after calling a PreparedPackage
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, PackageDownloadTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(GetDefaultPolygon());

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&$filter=Classification+eq+'Terrain'"));
    //request only terrain to limit the amount of responses

    cswBench->FilterSpatialEntity();

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageId());
    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageFile());

    WCharCP packageFile = cswBench->GetPackageFileName().GetName();
    ASSERT_TRUE(BeFileName::DoesPathExist(packageFile));
    BeFileName::EmptyAndRemoveDirectory(packageFile);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
// From WSG to Package to download
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, COMPLETETest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(GetDefaultPolygon());

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails());

    cswBench->FilterSpatialEntity();

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageId());
    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackageFile());

    BeFileName packageFile = cswBench->GetPackageFileName();
    ASSERT_TRUE(BeFileName::DoesPathExist(packageFile.GetName()));

    BeXmlStatus status = BEXML_Success;
    /*BeXmlDomPtr pDom = BeXmlDom::CreateAndReadFromFile(status, packageFile.GetName());
    ASSERT_EQ(BEXML_Success, status);
     
    status = pDom->SchemaValidate(GetXsd_2_0().c_str());
    if(BEXML_Success != status) // report error string first.
        {
        WString errorString;
        BeXmlDom::GetLastErrorString (errorString);
        ASSERT_STREQ(L"", errorString.c_str());
        }
    ASSERT_EQ(BEXML_Success, status);*/

    RealityDataDownload::Link_File_wMirrors_wSisters downloadOrder = RealityConversionTools::PackageFileToDownloadOrder(packageFile);

    ASSERT_TRUE(downloadOrder.size() >= 1);
    BeFileName::BeDeleteFile(packageFile.GetName());

    //make sure none of the files are already in the cache
//#pragma omp parallel for
    for(RealityPlatform::RealityDataDownload::mirrorWSistersVector mirrorVec : downloadOrder)
        for(RealityPlatform::RealityDataDownload::sisterFileVector sisterVec : mirrorVec)
            for(RealityPlatform::RealityDataDownload::url_file_pair ufPair : sisterVec)
                BeFileName::BeDeleteFile(ufPair.m_filePath.c_str());

    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(downloadOrder);
    ASSERT_TRUE(pDownload != NULL);
    
    pDownload->SetCertificatePath(GetPemLocation());
    RealityDataDownload::DownloadReport* report = pDownload->Perform();
    
    ASSERT_TRUE(report != nullptr);

    Utf8String reportString;
    report->ToXml(reportString);
    BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString(status, reportString.c_str());
    ASSERT_TRUE(status == BeXmlStatus::BEXML_Success);
    int fileCount = 0;
    Utf8String attributeName;
    WString attribute;
    BeFile file;
    uint64_t actualSize, statedSize;

    while (IBeXmlReader::ReadResult::READ_RESULT_Success == (reader->ReadTo(IBeXmlReader::NodeType::NODE_TYPE_Element)))
        {
        Utf8String xmlNodeName;
        reader->GetCurrentNodeName(xmlNodeName);
        IBeXmlReader::NodeType nodeType = reader->GetCurrentNodeType();
        if (IBeXmlReader::NodeType::NODE_TYPE_Element != nodeType
            || 0 == xmlNodeName.CompareTo("File"))
            {
            fileCount++;
            reader->ReadToNextAttribute(&attributeName, &attribute); //FileName
            ASSERT_TRUE(attributeName.CompareToI("FileName") == 0);
            ASSERT_TRUE(BeFileName::DoesPathExist(attribute.c_str()));
            ASSERT_TRUE(BeFileStatus::Success == file.Open(attribute.c_str(), BeFileAccess::Read));
            reader->ReadToNextAttribute(&attributeName, &attribute); //url
            reader->ReadToNextAttribute(&attributeName, &attribute); //filesize
            ASSERT_TRUE(attributeName.CompareToI("filesize") == 0);
            BentleyStatus ret;
            statedSize = BeStringUtilities::ParseUInt64(Utf8String(attribute.c_str()).c_str(), &ret);
            ASSERT_TRUE(BentleyStatus::SUCCESS == ret);
            ASSERT_TRUE(BeFileStatus::Success == file.GetSize(actualSize));
            ASSERT_TRUE(actualSize == statedSize);
            file.Close();
            }
        }

    // clear the filesys
//#pragma omp parallel for
    for (RealityPlatform::RealityDataDownload::mirrorWSistersVector mirrorVec : downloadOrder)
        for (RealityPlatform::RealityDataDownload::sisterFileVector sisterVec : mirrorVec)
            for (RealityPlatform::RealityDataDownload::url_file_pair ufPair : sisterVec)
                BeFileName::BeDeleteFile(ufPair.m_filePath.c_str());
    ASSERT_TRUE(fileCount >= 4);
    }
