//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityAdmin/ContextServicesTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST

#include <windows.h>

#include <Bentley/BeTest.h>
#include <Bentley/BeFile.h>
#include <RealityAdmin/ContextServicesWorkbench.h>
#include <RealityPlatform/RealityConversionTools.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

#define BOX_ELLIPSE  0 
#define BOX_RECT     1 

#define CCH_MAXLABEL 80 
#define CX_MARGIN    12 

/*typedef struct tagLABELBOX 
    {  // box 
    RECT rcText;    // coordinates of rectangle containing text 
    BOOL fSelected; // TRUE if the label is selected 
    BOOL fEdit;     // TRUE if text is selected 
    int nType;      // rectangular or elliptical 
    int ichCaret;   // caret position 
    int ichSel;     // with ichCaret, delimits selection 
    int nXCaret;    // window position corresponding to ichCaret 
    int nXSel;      // window position corresponding to ichSel 
    int cchLabel;   // length of text in atchLabel 
    TCHAR atchLabel[CCH_MAXLABEL];
    } LABELBOX, *PLABELBOX;*/

//=====================================================================================
//! @bsiclass                          Spencer.Mason                            10/2016
//=====================================================================================
class ContextServicesTestFixture : public testing::Test
    {
    public:
	Utf8String GetToken()
        {
        Utf8String retString = "";
        WChar exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);

        WString exeDir = exePath;
        size_t pos = exeDir.find_last_of(L"/\\");
        exeDir = exeDir.substr(0, pos + 1);

        BeFileName testPath(exeDir);
        testPath.AppendToPath(L"TokenExtractor.exe");

        std::system(testPath.GetNameUtf8().c_str());

        HGLOBAL   hglb;
        LPTSTR    lptstr;

        if (!IsClipboardFormatAvailable(CF_TEXT))
            return "";
        if (!OpenClipboard(NULL))
            return "";

        hglb = GetClipboardData(CF_TEXT);
        if (hglb != NULL)
            {
            lptstr = (LPTSTR)GlobalLock(hglb);
            if (lptstr != NULL)
                {
                retString = Utf8String(lptstr);
                GlobalUnlock(hglb);
                }
            }
        CloseClipboard();

        return retString;
        }
        
    };

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
TEST_F(ContextServicesTestFixture, BasicTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

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
    
    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(filterPolygon);

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails());

    SpatioTemporalDatasetPtr dataset = SpatioTemporalDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

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

    cswBench->FilterSpatialEntity([](SpatioTemporalDataPtr entity) -> bool { Json::Value provider = entity->GetValueFromJson("DataProvider"); return provider.isString() ? provider.asString().EqualsI("Amazon Landsat 8") : false; });

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

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackage());
    }

TEST_F(ContextServicesTestFixture, SelectTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

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

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(filterPolygon);

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

TEST_F(ContextServicesTestFixture, FilterTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

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

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(filterPolygon);

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&$filter=Classification+eq+'Terrain'"));

    SpatioTemporalDatasetPtr dataset = SpatioTemporalDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i == 0);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 1);
    }

TEST_F(ContextServicesTestFixture, USGSTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

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

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(filterPolygon);

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails());

    SpatioTemporalDatasetPtr dataset = SpatioTemporalDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i >= 458);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 9);
    }

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

    SpatioTemporalDatasetPtr dataset = SpatioTemporalDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i >= 144);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 1);
    }

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

    SpatioTemporalDatasetPtr dataset = SpatioTemporalDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t i = dataset->GetImageryGroup().size();

    ASSERT_TRUE(i >= 178);

    i = dataset->GetTerrainGroup().size();

    ASSERT_TRUE(i >= 1);
    }

/*TEST_F(ContextServicesTestFixture, PagingTest)
{
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

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

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(filterPolygon);

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails());

    SpatioTemporalDatasetPtr dataset = SpatioTemporalDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t fullSetSize = dataset->GetImageryGroup().size() + dataset->GetTerrainGroup().size();

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&$top=100"));

    SpatioTemporalDatasetPtr dataset = SpatioTemporalDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t topSize = dataset->GetImageryGroup().size() + dataset->GetTerrainGroup().size();
    
    ASSERT_TRUE(topSize <= 100); // < if there are less than 100 results 

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&$skip=100"));

    SpatioTemporalDatasetPtr dataset = SpatioTemporalDataset::CreateFromJson(cswBench->GetSpatialEntityWithDetailsJson().c_str());

    size_t skipSize = dataset->GetImageryGroup().size() + dataset->GetTerrainGroup().size();

    ASSERT_TRUE(skipSize >= (fullSetSize - 100)); // > if there are less than 100 results in the full set
}*/

TEST_F(ContextServicesTestFixture, SourceTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

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

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(filterPolygon);

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

    cswBench->FilterSpatialEntity([](SpatioTemporalDataPtr entity) -> bool { Json::Value provider = entity->GetValueFromJson("DataProvider"); return provider.isString() ? !provider.asString().ToUpper().EqualsI("USGS") : false; });

    selectedIds = cswBench->GetSelectedIds();

    low = selectedIds[ResolutionCriteria::Low];
    medium = selectedIds[ResolutionCriteria::Medium];
    high = selectedIds[ResolutionCriteria::High];

    ASSERT_TRUE(low.size() == lowSize);
    ASSERT_TRUE(medium.size() == midSize);
    ASSERT_TRUE(high.size() == highSize);
    }

TEST_F(ContextServicesTestFixture, PackageIdTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

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

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(filterPolygon);

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&$filter=Classification+eq+'Terrain'")); 
    //request only terrain to limit the amount of responses

    cswBench->FilterSpatialEntity();

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackage());

    ASSERT_TRUE(cswBench->GetInstanceId().Contains(".xrdp"));
    }

TEST_F(ContextServicesTestFixture, PackageDownloadTest)
    {
    Utf8String token = GetToken();
    ASSERT_TRUE(token.length() > 100);

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

    RealityPlatform::GeoCoordinationParams params = RealityPlatform::GeoCoordinationParams(filterPolygon);

    RealityPlatform::ContextServicesWorkbench* cswBench = RealityPlatform::ContextServicesWorkbench::Create(token, params);

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadSpatialEntityWithDetails("&$filter=Classification+eq+'Terrain'"));
    //request only terrain to limit the amount of responses

    cswBench->FilterSpatialEntity();

    ASSERT_TRUE(BentleyStatus::SUCCESS == cswBench->DownloadPackage());

    WCharCP packageFile = cswBench->GetPackageFileName().GetName();
    ASSERT_TRUE(BeFileName::DoesPathExist(packageFile));
    BeFileName::EmptyAndRemoveDirectory(packageFile);
    }
