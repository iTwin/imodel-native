//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/RealityDataDownloadTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST


#include <Bentley/BeTest.h>
#include <RealityPlatform/RealityDataDownload.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsiclass                          Spencer.Mason                            10/2016
//=====================================================================================
class RealityDataDownloadTestFixture : public testing::Test
    {
    public:
        WCharCP directory = L"D:\\RealityDataDownloadTestDirectory";
        void InitTestDirectory(WCharCP directoryname)
            {
            if(BeFileName::DoesPathExist(directoryname))
                BeFileName::EmptyAndRemoveDirectory(directoryname);
            BeFileName::CreateNewDirectory(directoryname);
            }

        //Before each test
        virtual void SetUp() 
        {
        InitTestDirectory(directory);
        }

        //After each test
        virtual void TearDown()
        {
        BeFileName::EmptyAndRemoveDirectory(directory);
        }
    };

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
TEST_F(RealityDataDownloadTestFixture, SimpleDownload)
    {
    AString urlUSGSLink = "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/19/IMG/ned19_n40x25_w075x75_pa_east_2006.zip";

    WString filename(directory);
    RealityDataDownload::ExtractFileName(filename, urlUSGSLink);

    bvector<std::pair<AString, WString>> simpleDlList = bvector<std::pair<AString, WString>>();
    simpleDlList.push_back(std::make_pair(urlUSGSLink, filename));

    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(simpleDlList);
    ASSERT_TRUE(pDownload != nullptr);

    pDownload->Perform();
    
    ASSERT_TRUE(BeFileName::DoesPathExist(filename.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
TEST_F(RealityDataDownloadTestFixture, SisterDownload)
    {
    bvector<AString> urlOSMLink =
        {
        "ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/19/IMG/ned19_n40x25_w075x75_pa_east_2006.zip",
        "http://overpass-api.de/api/map?bbox=-112.1320,40.5292,-111.5200,40.8019",
        };

    WString filename1(directory);
    RealityDataDownload::ExtractFileName(filename1, urlOSMLink[0]);
    WString filename2(directory);
    RealityDataDownload::ExtractFileName(filename2, urlOSMLink[1]);

    RealityDataDownload::Link_File_wMirrors_wSisters sisterDlList = bvector<bvector<bvector<std::pair<AString, WString>>>>();
    RealityDataDownload::mirrorWSistersVector mirrorGroup = bvector<bvector<std::pair<AString, WString>>>();
    RealityDataDownload::sisterFileVector ufPair = bvector<std::pair<AString, WString>>();

    for (size_t i = 0; i < urlOSMLink.size(); ++i)
        {
        wchar_t filename[1024];
        swprintf(filename, 1024, L"D:\\RealityDataDownloadTestDirectory\\OsmFile_%2llu.osm", i);

        ufPair.push_back(std::make_pair(urlOSMLink[i], WString(filename)));
        }
    mirrorGroup.push_back(ufPair);
    sisterDlList.push_back(mirrorGroup);
    
    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(sisterDlList);
    ASSERT_TRUE(pDownload != nullptr);

    pDownload->Perform();
    ASSERT_TRUE(BeFileName::DoesPathExist(ufPair[0].second.c_str()));
    ASSERT_TRUE(BeFileName::DoesPathExist(ufPair[1].second.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
/*TEST_F(RealityDataDownloadTestFixture, MirrorDownload)
    {
    bvector<AString> urlOSMLink =
        {
        "http://api.openstreetmap.org/api/0.6/map?bbox=-112.132,40.5292,-111.52,40.8019",
        "http://overpass-api.de/api/map?bbox=-112.1320,40.5292,-111.5200,40.8019",
        };

    WString filename1(directory);
    RealityDataDownload::ExtractFileName(filename1, urlUSGSLink[0]);
    WString filename2(directory);
    RealityDataDownload::ExtractFileName(filename2, urlUSGSLink[1]);

    RealityDataDownload::sisterFileVector sisterDlList = bvector<bvector<std::pair<AString, WString>>>();

    for (size_t i = 0; i < urlOSMLink.size(); ++i)
        {
        wchar_t filename[1024];
        swprintf(filename, 1024, L"%lsOsmFile_%2llu.osm", sOutputFolder.c_str(), i);

        wMirrors = bvector<std::pair<AString, WString>>();
        wMirrors.push_back(std::make_pair("http://api.openstreetmap.org/api/0.6/map?ddox=-112.132,40.5292,-111.52,40.8019", WString(filename))); //url with typo, to force use of mirror
        wMirrors.push_back(std::make_pair(urlOSMLink[i], WString(filename)));
        wSisters = bvector<bvector<std::pair<AString, WString>>>();
        wSisters.push_back(wMirrors);
        urlList.push_back(wSisters); //mirror file test
        }
    }
    */
