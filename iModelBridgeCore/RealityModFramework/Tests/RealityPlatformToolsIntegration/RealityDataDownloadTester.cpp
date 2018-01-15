//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatformToolsIntegration/RealityDataDownloadTester.cpp $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST

#include <windows.h>

#include <Bentley/BeTest.h>
#include <Bentley/BeFile.h>
#include <RealityPlatformTools/RealityDataDownload.h>
#include "../Common/RealityModFrameworkTestsCommon.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsiclass                          Spencer.Mason                            10/2016
//=====================================================================================
class RealityDataDownloadTestFixture : public testing::Test
    {
    public:
        WCharCP GetDirectory()
            {
            BeFileName outDir;
            BeTest::GetHost().GetTempDir (outDir);
            outDir.AppendToPath(L"RealityDataDownloadTestDirectory");
            return outDir;
            }

        BeFileName GetPemLocation()
            {
            BeFileName outDir;
            BeTest::GetHost().GetDgnPlatformAssetsDirectory (outDir);
            outDir.AppendToPath(L"http").AppendToPath(L"cabundle.pem");
            return outDir;
            }

        void InitTestDirectory(WCharCP directoryname)
            {
            if(BeFileName::DoesPathExist(directoryname))
                BeFileName::EmptyAndRemoveDirectory(directoryname);
            BeFileName::CreateNewDirectory(directoryname);
            }
    };

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
TEST_F(RealityDataDownloadTestFixture, SimpleDownload)
    {
    AString urlUSGSLink = "https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N16E055.hgt.zip";
    WString directory(GetDirectory());
    directory.append(L"SimpleDownload");
    InitTestDirectory(directory.c_str());

    WString filename(directory);
    RealityDataDownload::ExtractFileName(filename, urlUSGSLink);

    bvector<RealityDataDownload::url_file_pair> simpleDlList = bvector<RealityDataDownload::url_file_pair>();
    simpleDlList.push_back(RealityDataDownload::url_file_pair(urlUSGSLink, filename));

    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(simpleDlList);
    ASSERT_TRUE(pDownload != nullptr);
    pDownload->SetCertificatePath(GetPemLocation());

    pDownload->SetTimeout(30L);
    pDownload->Perform();
    
    ASSERT_TRUE(BeFileName::DoesPathExist(filename.c_str()));
    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
TEST_F(RealityDataDownloadTestFixture, SisterDownload)
    {
    bvector<AString> urlOSMLink =
        {
        "https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N27E050.hgt.zip",
        "https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N27W017.hgt.zip",
        };

    WString directory(GetDirectory());
    directory.append(L"SisterDownload");
    InitTestDirectory(directory.c_str());

    WString filename1(directory);
    RealityDataDownload::ExtractFileName(filename1, urlOSMLink[0]);
    WString filename2(directory);
    RealityDataDownload::ExtractFileName(filename2, urlOSMLink[1]);

    RealityDataDownload::Link_File_wMirrors_wSisters sisterDlList = bvector<bvector<bvector<RealityDataDownload::url_file_pair>>>();
    RealityDataDownload::mirrorWSistersVector mirrorGroup = bvector<bvector<RealityDataDownload::url_file_pair>>();
    RealityDataDownload::sisterFileVector ufPair = bvector<RealityDataDownload::url_file_pair>();

    for (size_t i = 0; i < urlOSMLink.size(); ++i)
        {
        wchar_t filename[1024];
        swprintf(filename, 1024, L"%ls\\OsmFile_%2llu.osm", directory.c_str(), i);

        ufPair.push_back(RealityDataDownload::url_file_pair(urlOSMLink[i], WString(filename)));
        }
    mirrorGroup.push_back(ufPair);
    sisterDlList.push_back(mirrorGroup);
    
    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(sisterDlList);
    ASSERT_TRUE(pDownload != nullptr);
    pDownload->SetCertificatePath(GetPemLocation());

    pDownload->SetTimeout(30L);
    pDownload->Perform();
    ASSERT_TRUE(BeFileName::DoesPathExist(ufPair[0].m_filePath.c_str()));
    ASSERT_TRUE(BeFileName::DoesPathExist(ufPair[1].m_filePath.c_str()));
    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
TEST_F(RealityDataDownloadTestFixture, MirrorDownload)
    {
    bvector<AString> mirrors =
        {
        "ftp://ftp.geogratis.gc.ca/pub/nrcan_rncan/image/canimage/50k/012/zz/badfilename.bip", //bad url, to force use of mirror
        "https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N29E049.hgt.zip",
        };

    WString directory(GetDirectory());
    directory.append(L"MirrorDownload");
    InitTestDirectory(directory.c_str());

    WString filename1(directory);
    RealityDataDownload::ExtractFileName(filename1, mirrors[0]);
    WString filename2(directory);
    RealityDataDownload::ExtractFileName(filename2, mirrors[1]);

    RealityDataDownload::Link_File_wMirrors mirrorDlList = bvector<bvector<RealityDataDownload::url_file_pair>>();
    RealityDataDownload::mirrorVector ufPair = bvector<RealityDataDownload::url_file_pair>();

    for (size_t i = 0; i < mirrors.size(); ++i)
        {
        wchar_t filename[1024];
        swprintf(filename, 1024, L"%ls\\OsmFile_%2llu.osm", directory.c_str(), i);

        ufPair.push_back(RealityDataDownload::url_file_pair(mirrors[i], WString(filename)));
        }

    mirrorDlList.push_back(ufPair);

    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(mirrorDlList);
    ASSERT_TRUE(pDownload != nullptr);
    pDownload->SetCertificatePath(GetPemLocation());

    pDownload->SetTimeout(30L);
    pDownload->Perform();
    ASSERT_TRUE(!BeFileName::DoesPathExist(ufPair[0].m_filePath.c_str()));
    ASSERT_TRUE(BeFileName::DoesPathExist(ufPair[1].m_filePath.c_str()));
    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    }

TEST_F(RealityDataDownloadTestFixture, DownloadCacheAndReport)
    {
    WString directory(GetDirectory());
    directory.append(L"DownloadCacheAndReport");
    InitTestDirectory(directory.c_str());

    WString cachename1(directory);
    RealityDataDownload::ExtractFileName(cachename1, "cache1.zip");
    BeFile cache1, cache2;
    cache1.Create(cachename1.c_str(), true);
    WString cachename2(directory);
    RealityDataDownload::ExtractFileName(cachename2, "cache2.zip");
    cache2.Create(cachename2.c_str(), true);

    ASSERT_TRUE(BeFileName::DoesPathExist(cachename1.c_str()));
    ASSERT_TRUE(BeFileName::DoesPathExist(cachename2.c_str()));

    WString filename1(directory);
    RealityDataDownload::ExtractFileName(filename1, L"can01.zip");
    WString filename2(directory);
    RealityDataDownload::ExtractFileName(filename2, L"can02.zip");
    WString filename3(directory);
    RealityDataDownload::ExtractFileName(filename3, L"can03.zip");

    bvector<bvector<RealityDataDownload::url_file_pair>> cacheTest1 =
        {
            {
            RealityDataDownload::url_file_pair("https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N16E055.hgt.zip", filename1),
            RealityDataDownload::url_file_pair("https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N27E050.hgt.zip", cachename1),
            RealityDataDownload::url_file_pair("https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N27W017.hgt.zip", cachename2)
            }
        };//dl first file, other 2 exist

    bvector<bvector<RealityDataDownload::url_file_pair>> cacheTest2 =
        {
            {
            RealityDataDownload::url_file_pair("https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N29E049.hgt.zip", cachename1),
            RealityDataDownload::url_file_pair("https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N37W025.hgt.zip", filename2),
            RealityDataDownload::url_file_pair("https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N16E055.hgt.zip", cachename2)
            }
        };//dl second file, other 2 exist

    bvector<bvector<RealityDataDownload::url_file_pair>> cacheTest3 =
        {
            {
            RealityDataDownload::url_file_pair("https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N27E050.hgt.zip", cachename1),
            RealityDataDownload::url_file_pair("https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N27W017.hgt.zip", cachename2),
            RealityDataDownload::url_file_pair("https://dds.cr.usgs.gov/srtm/version2_1/SRTM3/Africa/N29E049.hgt.zip", filename3)
            }
        };//dl last file, other 2 exist

    RealityDataDownload::Link_File_wMirrors_wSisters sisterDlList = bvector<bvector<bvector<RealityDataDownload::url_file_pair>>>();

    sisterDlList.push_back(cacheTest1);
    sisterDlList.push_back(cacheTest2);
    sisterDlList.push_back(cacheTest3);

    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(sisterDlList);
    ASSERT_TRUE(pDownload != nullptr);
    pDownload->SetCertificatePath(GetPemLocation());

    pDownload->SetTimeout(30L);
    RealityDataDownload::DownloadReport* report = pDownload->Perform();
    ASSERT_TRUE(BeFileName::DoesPathExist(filename1.c_str()));
    ASSERT_TRUE(BeFileName::DoesPathExist(filename2.c_str()));
    ASSERT_TRUE(BeFileName::DoesPathExist(filename3.c_str()));

    BeXmlStatus status;
    Utf8String reportString;
    report->ToXml(reportString);
    BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString(status, reportString.c_str());
    ASSERT_TRUE(status == BeXmlStatus::BEXML_Success);
    int fileCount = 0;
    /*WString innerXml;
    reader->GetCurrentNodeString(innerXml);*/

    while (IBeXmlReader::ReadResult::READ_RESULT_Success == (reader->ReadTo(IBeXmlReader::NodeType::NODE_TYPE_Element)))
        {
        Utf8String xmlNodeName;
        reader->GetCurrentNodeName(xmlNodeName);
        IBeXmlReader::NodeType nodeType = reader->GetCurrentNodeType();
        if (IBeXmlReader::NodeType::NODE_TYPE_Element != nodeType
            || 0 == xmlNodeName.CompareTo("File"))
            fileCount++;
        }

    ASSERT_TRUE(fileCount == 3);
    cache1.Close();
    cache2.Close();
    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
TEST_F(RealityDataDownloadTestFixture, UnzipTest)
    {
    WString zipString = RealityModFrameworkTestsUtils::GetTestDataPath(L"Testdata\\RealityPlatformTools\\download.zip");

    WString outString = GetDirectory(); 
    outString.append(L"Unzip/");
    InitTestDirectory(outString.c_str());
    WString dl = outString;
    dl.append(L"download");
    InitTestDirectory(dl.c_str());

    outString.ReplaceAll(L"\\",L"/");

    EXPECT_TRUE(RealityDataDownload::UnZipFile(zipString, outString));

    BeFileName::EmptyAndRemoveDirectory(outString.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            10/2016
//-------------------------------------------------------------------------------------
//deactivated because the UUID is not stable
/*TEST_F(RealityDataDownloadTestFixture, AuthInfoHeader)
    {
    AString dl1 = "https://scihub.copernicus.eu/dhus/odata/v1/Products('16902fd3-f323-4014-a950-853ac602e22f')/Nodes('S1A_IW_SLC__1SDV_20141101T165548_20141101T165616_003091_0038AA_558F.SAFE')/Nodes('manifest.safe')/$value";

    AString dl2 = "https://scihub.copernicus.eu/dhus/odata/v1/Products('16902fd3-f323-4014-a950-853ac602e22f')/Nodes('S1A_IW_SLC__1SDV_20141101T165548_20141101T165616_003091_0038AA_558F.SAFE')/Nodes('preview')/Nodes('quick-look.png')/$value";

    WString directory(GetDirectory());
    directory.append(L"AuthInfoHeader");
    InitTestDirectory(directory.c_str());

    WString filename1(directory);
    RealityDataDownload::ExtractFileName(filename1, L"manifest.safe");
    WString filename2(directory);
    RealityDataDownload::ExtractFileName(filename2, L"quick-look.png");

    RealityDataDownload::Link_File_wMirrors dlList = bvector<bvector<RealityDataDownload::url_file_pair>>();
    RealityDataDownload::mirrorVector ufPair = bvector<RealityDataDownload::url_file_pair>();

    RealityDataDownload::AuthInfo auth = RealityDataDownload::AuthInfo();
    auth.headers.push_back("Authorization: Basic U3BlbmNlckJlbnRsZXk6c0JlbnRsZXkx==");
    RealityDataDownload::DownloadCap cap = RealityDataDownload::DownloadCap("scihub", 1, auth);

    ufPair.push_back(RealityDataDownload::url_file_pair(dl1, filename1, "", &cap));
    dlList.push_back(ufPair);

    ufPair.clear();
    ufPair.push_back(RealityDataDownload::url_file_pair(dl2, filename2, "", &cap));
    dlList.push_back(ufPair);

    RealityDataDownloadPtr pDownload = RealityDataDownload::Create(dlList);
    ASSERT_TRUE(pDownload != nullptr);
    pDownload->SetCertificatePath(GetPemLocation());

    pDownload->SetTimeout(30L);
    pDownload->Perform();
    ASSERT_TRUE(BeFileName::DoesPathExist(filename1.c_str()));
    ASSERT_TRUE(BeFileName::DoesPathExist(filename2.c_str()));
    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    }*/