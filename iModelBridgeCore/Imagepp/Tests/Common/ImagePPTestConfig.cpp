//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "ImagePPTestConfig.h"
#include <ImagePP/all/h/HRFFileFormats.h>


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

    // Create files with the same dummy date so we have a consistent md5 hash value.
    HRFRasterFileFactory::GetInstance()->__test__hijackFileCreationTime(true);

    BeFileName assetDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetDirectory);

    BeFileName geoCoordDataDir(assetDirectory);
    geoCoordDataDir.AppendToPath(L"DgnGeoCoord");
    GeoCoordinates::BaseGCS::Initialize(geoCoordDataDir.c_str());

    BeFileName configFile(assetDirectory);
    configFile.AppendToPath(L"ImagePPTest.config.xml");

    BeXmlStatus status;
    BeXmlDomPtr pXmlDoc = BeXmlDom::CreateAndReadFromFile(status, configFile.c_str(), nullptr);
    if (!pXmlDoc.IsValid() || BEXML_Success != status)
        {
        fprintf(stderr, "\n\n[ERROR] Failed to read: %s\n\n", configFile.GetNameUtf8().c_str());
        return;
        }

    // -- Source directory
    BeXmlNodeP pSourceDirNode = nullptr;
    if (BEXML_Success == pXmlDoc->SelectNode(pSourceDirNode, "/ImagePPTest/FileFormatTester/Source", nullptr, BeXmlDom::NODE_BIAS_First))
        {
        bool enable = false;
        pSourceDirNode->GetAttributeBooleanValue(enable, "Enable");
        if (enable)
            {
            pSourceDirNode->GetAttributeStringValue(m_sourceDir, "Dir");
            if (!BeFileName::DoesPathExist(m_sourceDir))
                {
                fprintf(stderr, "\n\n[ERROR] Failed Source path invalid: %ls\n\n", m_sourceDir.c_str());
                BeAssert(true);
                }
            }
        }

    // -- Baseline and validation.
    BeXmlNodeP pBaseLineDirNode = nullptr;
    if (BEXML_Success == pXmlDoc->SelectNode(pBaseLineDirNode, "/ImagePPTest/FileFormatTester/BaseLine", nullptr, BeXmlDom::NODE_BIAS_First))
        {
        bool enable = false;
        pBaseLineDirNode->GetAttributeBooleanValue(enable, "Enable");
        if(enable)
            pBaseLineDirNode->GetAttributeStringValue(m_baselineDir, "Dir");

        m_md5Validation = false;
        BeXmlNodeP pMd5Node = nullptr;
        if (BEXML_Success == pXmlDoc->SelectChildNodeByName(pMd5Node, *pBaseLineDirNode, "Md5", BeXmlDom::NODE_BIAS_First))
            pMd5Node->GetAttributeBooleanValue(m_md5Validation, "Enable");
                
        m_validateDuration = false;
        BeXmlNodeP pDurationNode = nullptr;
        if (BEXML_Success == pXmlDoc->SelectChildNodeByName(pDurationNode, *pBaseLineDirNode, "Duration", BeXmlDom::NODE_BIAS_First))
            {
            pDurationNode->GetAttributeBooleanValue(m_validateDuration, "Enable");
            if (m_validateDuration)
                {
                pDurationNode->GetAttributeDoubleValue(m_toleranceRatio, "ToleranceRatio");
                pDurationNode->GetAttributeUInt64Value(m_durationThresholdMs, "ThresholdMs");
                }
            }
        }

    // -- Evaluate output dir, default to BeTest output.
    BeTest::GetHost().GetOutputRoot(m_fileFormatOutputDir);

    BeXmlNodeP pOutputDirNode = nullptr;
    if (BEXML_Success == pXmlDoc->SelectNode(pOutputDirNode, "/ImagePPTest/FileFormatTester/OutputOverride", nullptr, BeXmlDom::NODE_BIAS_First))
        {
        bool enable = false;
        pOutputDirNode->GetAttributeBooleanValue(enable, "Enable");
        if (enable)
            pOutputDirNode->GetAttributeStringValue(m_fileFormatOutputDir, "Dir");
        }

    // Add a sub folder so FileFormatTests output are not mixed with others unit tests.
    if (!m_fileFormatOutputDir.IsEmpty())
        m_fileFormatOutputDir.AppendToPath(L"FFT");
    if (!m_baselineDir.IsEmpty())
        m_baselineDir.AppendToPath(L"FFT");

    // Make sure all our paths end with a separator
    if (!m_sourceDir.IsEmpty())
        m_sourceDir.AppendSeparator();
    if (!m_fileFormatOutputDir.IsEmpty())
        m_fileFormatOutputDir.AppendSeparator();
    if (!m_baselineDir.IsEmpty())
        m_baselineDir.AppendSeparator();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
ImagePPTestConfig::~ImagePPTestConfig()
    {
    // Never called. 
    }


