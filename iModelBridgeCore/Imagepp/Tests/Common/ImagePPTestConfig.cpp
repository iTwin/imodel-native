//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/Common/ImagePPTestConfig.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "ImagePPTestConfig.h"
#include <ImagePP/all/h/HRFFileFormats.h>
#include <Bentley/BeDirectoryIterator.h>


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
struct TestImageppLibAdmin : ImagePP::ImageppLibAdmin
    {
    DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)

    virtual BentleyStatus _GetDefaultTempDirectory(BeFileName& tempFileName) const override
        {
        BeTest::GetHost().GetTempDir(tempFileName);
        return BentleyStatus::SUCCESS;
        }

    TestImageppLibAdmin(){}
    virtual ~TestImageppLibAdmin() {}
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
ImageppLibAdmin& ImagePPTestConfig::_SupplyImageppLibAdmin()
    {
    return *new TestImageppLibAdmin();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
void ImagePPTestConfig::_RegisterFileFormat() { REGISTER_SUPPORTED_FILEFORMAT }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
/*static*/ ImagePPTestConfig& ImagePPTestConfig::GetConfig()
    {
    static ImagePPTestConfig* s_pInstance = new ImagePPTestConfig();
    return *s_pInstance;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
ImagePPTestConfig::ImagePPTestConfig()
    {
    // Required for sqlang stuff....
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    if (!tempDir.DoesPathExist())
        {
        BeFileNameStatus statusDir = BeFileName::CreateNewDirectory(tempDir.c_str());
        BeAssert(statusDir == BeFileNameStatus::Success);
        }
    BeSQLite::BeSQLiteLib::Initialize(tempDir);

    //Initialize host
    ImagePP::ImageppLib::Initialize(*this);

    BeFileName assetDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetDirectory);

    BeFileName geoCoordDataDir(assetDirectory);
    geoCoordDataDir.AppendToPath(L"DgnGeoCoord");
    GeoCoordinates::BaseGCS::Initialize(geoCoordDataDir.c_str());

    BeFileName configFile(assetDirectory);
    configFile.AppendToPath(L"ImagePPTest.config.xml");

    BeXmlStatus status;
    BeXmlDomPtr pXmlDoc = BeXmlDom::CreateAndReadFromFile(status, configFile.c_str(), nullptr);
    if (pXmlDoc.IsValid() && BEXML_Success == status)
        {
        BeXmlNodeP pSourceDirNode = nullptr;
        if (BEXML_Success == pXmlDoc->SelectNode(pSourceDirNode, "/ImagePPTest/FileFormatTester/Source", nullptr, BeXmlDom::NODE_BIAS_First))
            {
            bool enable = false;
            pSourceDirNode->GetAttributeBooleanValue(enable, "Enable");
            if (enable)
                {
                pSourceDirNode->GetContent(m_sourceDir);
                m_sourceFileList = BuildFileList(m_sourceDir);
                }
            }

        BeXmlNodeP pBaseLineDirNode = nullptr;
        if (BEXML_Success == pXmlDoc->SelectNode(pBaseLineDirNode, "/ImagePPTest/FileFormatTester/BaseLine", nullptr, BeXmlDom::NODE_BIAS_First))
            {
            bool enable = false;
            pBaseLineDirNode->GetAttributeBooleanValue(enable, "Enable");
            if(enable)
                pBaseLineDirNode->GetContent(m_baselineDir);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
ImagePPTestConfig::~ImagePPTestConfig()
    {
    // Never called. 
    }

/*---------------------------------------------------------------------------------**//**
* Return a vector with all the paths to the rasters or directories.
* @bsimethod                                             Laurent.Robert-Veillette 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::wstring> ImagePPTestConfig::BuildFileList(BeFileNameCR directory)
    {
    std::list<std::wstring> directoryList;
    bvector<BeFileName> fileList;

    const WString glob = L"*";

    BeDirectoryIterator::WalkDirsAndMatch(fileList, directory, glob.c_str(), true);

    // Scan the fileList and skipping not supported rasters and folders
    for (auto& actualName : fileList)
        {
        if (actualName.IsDirectory() ||
            actualName.ContainsI(L"thumb.db") ||                // Ignore windows thumbnail.
            actualName.ContainsI(L"\\PSS\\")                    // Skip PSS for now.
            )
            continue;

        directoryList.push_back(actualName.GetName());
        }

    return directoryList;
    }