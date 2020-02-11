//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <ImagePP/h/ImageppAPI.h>
#include <ImagePP/all/h/HRFFileFormats.h>
#include <ImagePP/all/h/HUTImportFromFileExportToFile.h>
#include <ImagePP/all/h/HUTImportFromRasterExportToFile.h>
#include "../Common/ImagePPTestConfig.h"
#include "ExporterTestFixture.h"

//#ifdef USE_GTEST        // TEST_P only available when using gtest.

#include <ImagePP/all/h/HUTClassIDDescriptor.h>
#include <Bentley/BeFileName.h>
#include <ImagePP/all/h/HPMPool.h>
#include <ImagePP/all/h/HRSObjectStore.h>
#include <ImagePP/all/h/HRFiTiffFile.h>
#include <ImagePP/all/h/HRFRasterFileExtender.h>


USING_NAMESPACE_IMAGEPP

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
class HRFTester : public ::testing::Test
    {
    protected:
    HRFTester() {};
    ~HRFTester() {};
    };

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
TEST_F(HRFTester, UniqueShortName)
    {
    ImagePPTestConfig::GetConfig().SetUp();
    HRFRasterFileFactory::CreatorsMap const& creators = HRFRasterFileFactory::GetInstance()->GetCreatorsMap(HFC_CREATE_ONLY);

    std::set<Utf8String> shortNameSet;

    for (auto& entry : creators)
        {
        auto insertResult = shortNameSet.insert(entry.second->GetShortName());
        ASSERT_TRUE(insertResult.second) << "not unique: " << entry.second->GetShortName();
        ASSERT_FALSE(entry.second->GetLabel().empty());  // maybe the sqlang file it not present?
        }
    }

/*---------------------------------------------------------------------------------**//**
* Class definition
* @bsimethod                                             Laurent.Robert-Veillette 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
class ExportAllTester : public ExporterTestFixture < ::testing::TestWithParam< ::std::tr1::tuple<std::wstring, HRFRasterFileCreator*> > >
    {
    public:
        ExportAllTester() {}
        virtual ~ExportAllTester() {};

        std::wstring const& GetSourceFilename() { return ::std::tr1::get<0>(GetParam()); }
        HRFRasterFileCreator& GetCreator() { return *::std::tr1::get<1>(GetParam()); }
 
    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  6/2016
    //----------------------------------------------------------------------------------------
    static std::list<HRFRasterFileCreator*> const& GetCreatorList()
        {
        static std::list<HRFRasterFileCreator*> s_creatorList;
        if (s_creatorList.empty())
            {
            // must init here because we called before SetUpTestCase/SetUp();
            ImagePPTestConfig::GetConfig().SetUp();
            HRFRasterFileFactory::CreatorsMap const& creators = HRFRasterFileFactory::GetInstance()->GetCreatorsMap(HFC_CREATE_ONLY);

            for (auto& entry : creators)
                {                
                if (HRFPWRasterFile::CLASS_ID == entry.first || // Causes a crash to export to this format.
                    HRFRawFile::CLASS_ID == entry.first      || // They cannot be reopened
                    HRFEpsFile::CLASS_ID == entry.first        // They cannot be reopened    
                    )
                    continue;

                s_creatorList.push_back(entry.second);
                }
            }

        return s_creatorList;
        }

        //----------------------------------------------------------------------------------------
        // @bsimethod                                                   Mathieu.Marchand  6/2016
        //----------------------------------------------------------------------------------------
        static std::list<std::wstring> const& GetSourceList()
            {
            static std::list<std::wstring> s_sourceList;
            static bool s_created = false;
            if (!s_created)
                {
                BeFileName sourceDir = ImagePPTestConfig::GetConfig().GetSourceDir();
                if (!sourceDir.IsEmpty())   // Empty mean disabled
                    {
                    s_sourceList.push_back(L"Images\\iTIFF\\pattern.itiff");
                    }

                s_created = true;
                }

            return s_sourceList;
            }

        // Sets up the stuff shared by all tests in this test case.
        //
        // Google Test will call Foo::SetUpTestCase() before running the first
        // test in test case Foo.  Hence a sub-class can define its own
        // SetUpTestCase() method to shadow the one defined in the super
        // class.
        static void SetUpTestCase() {}

        // Tears down the stuff shared by all tests in this test case.
        //
        // Google Test will call Foo::TearDownTestCase() after running the last
        // test in test case Foo.  Hence a sub-class can define its own
        // TearDownTestCase() method to shadow the one defined in the super
        // class.
        static void TearDownTestCase() {}

    private:
    };


/*---------------------------------------------------------------------------------**//**
* Format the name of the file with the specific options of the export
* @bsimethod                                         Laurent.Robert-Veillette     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName s_GetProfileID(HCLASS_ID pixelTypeClassKey, HCLASS_ID codecClassKey, HRFBlockType blockType)
    {
    Utf8String ComposedFilename;

    ComposedFilename += HUTClassIDDescriptor::GetInstance()->GetClassCodePixelType(pixelTypeClassKey);
    ComposedFilename += HUTClassIDDescriptor::GetInstance()->GetClassCodeCodec(codecClassKey);

    switch (blockType.m_BlockType)
        {
        case HRFBlockType::LINE:
            ComposedFilename += "L";
            break;

        case HRFBlockType::TILE:
            ComposedFilename += "T";
            break;

        case HRFBlockType::STRIP:
            ComposedFilename += "S";
            break;

        case HRFBlockType::IMAGE:
            ComposedFilename += "I";
            break;
        default:
            BeAssert(!"unknown HRFBlockType");
            break;
        }
    

    return BeFileName(ComposedFilename);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
TEST_P(ExportAllTester, ToAllFormats)
    {
    //&&MM delete cache if we created one.
    //    - skip some format 
    //    - Skip some pixeltype in binary case.  Maybe it deserves its own test. BinaryToAllBinaryFormats
    //    -  Add a dump with info of build.  Source location, build type, date of run... do it in base 'ExporterTestFixture'.

    auto& config = ImagePPTestConfig::GetConfig();

    BeFileName sourceDir = config.GetSourceDir();
    if (sourceDir.IsEmpty())   // Empty mean disabled
        return;
    
    HRFRasterFileCreator& creator = GetCreator();
    ASSERT_TRUE(creator.GetSupportedAccessMode().m_HasCreateAccess);
   
    BeFileName sourceFilename(sourceDir);
    sourceFilename.AppendToPath(GetSourceFilename().c_str());
    ASSERT_TRUE(sourceFilename.DoesPathExist());

    WString outExtension(creator.GetDefaultExtension().c_str(), BentleyCharEncoding::Utf8);
    if (outExtension.StartsWith(L"*."))
        outExtension.erase(0, 2);

    BeFileName outputRoot(config.GetFileFormatOutputDir());
    ASSERT_FALSE(outputRoot.IsEmpty());

    BeFileName baseRelativeOutName("ExportToAll\\");
    baseRelativeOutName.AppendUtf8(creator.GetShortName().c_str());
    baseRelativeOutName += L"_";
    baseRelativeOutName += sourceFilename.GetFileNameAndExtension();

    HFCPtr<HFCURL> pSourceUrl = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + sourceFilename.GetNameUtf8());

    printf("[ TRACE    ]   Exporting: %s\n", creator.GetLabel().c_str());

    try
        {
        HFCPtr<HRFRasterFile> pRasterFileSource = HRFRasterFileFactory::GetInstance()->OpenFile(pSourceUrl, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
        ASSERT_TRUE(pRasterFileSource.GetPtr() != nullptr);
        
        // *** Pixeltype
        HFCPtr<HRFRasterFileCapabilities> capabilities = creator.GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID, HFC_CREATE_ONLY);
        for (uint32_t pixelTypeIndex=0; pixelTypeIndex < capabilities->CountCapabilities(); ++pixelTypeIndex)
            {
            HFCPtr<HRFPixelTypeCapability> pCurrentPixelType = (HFCPtr<HRFPixelTypeCapability>&)(capabilities->GetCapability(pixelTypeIndex));

            ASSERT_TRUE(pCurrentPixelType->GetAccessMode().m_HasCreateAccess);

            // *** Codec
            HFCPtr<HRFRasterFileCapabilities> codecCaps = pCurrentPixelType->GetCodecCapabilities();

            for (uint32_t codecIndex=0; codecIndex < codecCaps->CountCapabilities(); ++codecIndex)
                {
                HFCPtr<HRFCodecCapability> pCurrentCodec = (HFCPtr<HRFCodecCapability>&) codecCaps->GetCapability(codecIndex);

                if (!pCurrentCodec->GetAccessMode().m_HasCreateAccess)
                    continue;

                // *** BlockType
                HFCPtr<HRFRasterFileCapabilities> blockCaps = pCurrentCodec->GetBlockTypeCapabilities();
                for (uint32_t blockIndex=0; blockIndex < blockCaps->CountCapabilities(); ++blockIndex)
                    {
                    HFCPtr<HRFBlockCapability> pCurrentBlock = (HFCPtr<HRFBlockCapability>&) blockCaps->GetCapability(blockIndex);

                    if (!pCurrentBlock->GetAccessMode().m_HasCreateAccess)
                        continue;

                    WString ID = s_GetProfileID(pCurrentPixelType->GetPixelTypeClassID(), pCurrentCodec->GetCodecClassID(), pCurrentBlock->GetBlockType());

                    BeFileName outRelativeName(baseRelativeOutName);
                    outRelativeName.AppendString(L"_");
                    outRelativeName.AppendString(ID.c_str());
                    outRelativeName.AppendExtension(outExtension.c_str());

                    BeFileName outFilename(outputRoot);
                    outFilename.AppendToPath(outRelativeName.c_str());

                    ASSERT_FALSE(outFilename.DoesPathExist()) << outFilename.c_str();

                    HFCPtr<HFCURL> pOutputUrl = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outFilename.GetNameUtf8());

                    HFCPtr<HGF2DCoordSys> pLogical = GetWorld()->GetCoordSysReference(pRasterFileSource->GetPageWorldIdentificator(0));
                    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(&GetPool(), pRasterFileSource, 0, pLogical);
                    ASSERT_TRUE(pStore != nullptr);
                    HFCPtr<HRARaster> pRaster(pStore->LoadRaster());
                    ASSERT_TRUE(pRaster != nullptr);

                    std::unique_ptr<HUTImportFromRasterExportToFile> exporter(new HUTImportFromRasterExportToFile(pRaster, *pRaster->GetEffectiveShape(), GetWorld()));
                    
                    exporter->SelectExportFilename(pOutputUrl);

                    exporter->SelectExportFileFormat(&creator);
                    ASSERT_EQ(&creator, exporter->GetSelectedExportFileFormat());
                                        
                    exporter->SelectPixelType(pCurrentPixelType->GetPixelTypeClassID());
                    ASSERT_EQ(pCurrentPixelType->GetPixelTypeClassID(), exporter->GetSelectedPixelType());

                    exporter->SelectCodec(pCurrentCodec->GetCodecClassID());
                    ASSERT_EQ(pCurrentCodec->GetCodecClassID(), exporter->GetSelectedCodec());

                    exporter->SelectBlockType(pCurrentBlock->GetBlockType());
                    ASSERT_EQ(pCurrentBlock->GetBlockType(), exporter->GetSelectedBlockType());

                    // *** Export the file.....
                    uint64_t startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
                    HFCPtr<HRFRasterFile> pFile = exporter->StartExport();
                    ASSERT_TRUE(pFile.GetPtr() != nullptr);                        
                    exporter.reset();
                    pFile = nullptr;                    
                    uint64_t endTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

                    // *** Generate export info.
                    RasterTestInfo outputInfo(outFilename);
                    outputInfo.SetExportDuration(endTime - startTime);
                    ASSERT_TRUE(outputInfo.ComputeMD5());
                    ASSERT_TRUE(outputInfo.Store());

                    // *** Validate against baseline.
                    if (config.CompareAgainsBaseline())
                        {
                        BeFileNameCR baseLineDir = config.GetBaselineDir();
                        ASSERT_TRUE(baseLineDir.DoesPathExist());

                        BeFileName baselineFilename(baseLineDir);
                        baselineFilename.AppendToPath(outRelativeName.c_str());

                        if (!baselineFilename.DoesPathExist())
                            FAIL() << "Baseline specified but file is missing : " << baselineFilename.GetNameUtf8().c_str();

                        RasterTestInfo baselineInfo(baselineFilename);

                        if (!baselineInfo.Load())
                            FAIL() << "Baseline info file is missing : " << baselineInfo.GetInfoFilename().c_str();

                        if(config.DoMd5())
                            EXPECT_STREQ(baselineInfo.GetMD5().c_str(), outputInfo.GetMD5().c_str()) << "MD5 check failed: " << outFilename.c_str();

                        if (config.ValidateExportDuration())
                            {
                            EXPECT_STREQ(baselineInfo.GetBuildType().c_str(), outputInfo.GetBuildType().c_str()) << "Build must be of same build type for duration validation";
                            EXPECT_STREQ(baselineInfo.GetComputerName().c_str(), outputInfo.GetComputerName().c_str()) << "Baseline must be from the same machine for duration validation";

                            if (outputInfo.GetExportDuration() > config.GetDuratationThreshold())
                                {
                                double exportDelta = (double) outputInfo.GetExportDuration() - (double) baselineInfo.GetExportDuration();
                                double exportRatio = exportDelta / baselineInfo.GetExportDuration();
                                EXPECT_LE(exportRatio, config.GetToleranceRatio())
                                    << "Base time: " << baselineInfo.GetExportDuration() << "ms New time: " << outputInfo.GetExportDuration() << "ms";
                                }
                            }
                        }
                    }
                }
            }
        }
    catch (HFCException& e)
        {
        FAIL() << e.GetExceptionMessage().c_str();
        }
    }

INSTANTIATE_TEST_CASE_P(FormatTests, ExportAllTester, 
                        ::testing::Combine(::testing::ValuesIn(ExportAllTester::GetSourceList()),
                                           ::testing::ValuesIn(ExportAllTester::GetCreatorList())));

// #else
// 
// #pragma message("Warning: Disabling ExportAllTester because TEST_P/INSTANTIATE_TEST_CASE_P are not available")

//#endif
