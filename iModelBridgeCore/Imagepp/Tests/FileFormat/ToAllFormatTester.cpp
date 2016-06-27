//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/FileFormat/ToAllFormatTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

#include <Imagepp/all/h/HUTClassIDDescriptor.h>
#include <Bentley/BeFileName.h>
#include <Imagepp/all/h/HPMPool.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HRFiTiffFile.h>
#include <Imagepp/all/h/HRFRasterFileExtender.h>


USING_NAMESPACE_IMAGEPP

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
                    HRFEpsFile::CLASS_ID == entry.first         // They cannot be reopened    
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
                    s_sourceList.push_back(L"Images\\iTIFF\\TA00T1A2\\TS1_32bits_None_Tile.iTIFF");
                    //s_sourceList.push_back(L"Images\\iTIFF\\1A02S142\\TS1_2C_Alpha_CCITT4_Strip.iTIFF");
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
static BeFileName s_GetProfileID(const HFCPtr<HRFRasterFile>&   pi_rpRasterFile,
                                 HCLASS_ID                      pixelTypeClassKey,
                                 HCLASS_ID                      codecClassKey,
                                 HRFBlockType                   blockType)
    {
    Utf8String ComposedFilename;

    HRFResolutionDescriptor ResolutionDescriptor(*(pi_rpRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)));

    //------------------------------------
    // Color space (Pixel Type)
    ComposedFilename += HUTClassIDDescriptor::GetInstance()->GetClassCodePixelType(pixelTypeClassKey);

    //------------------------------------
    // Compression Codec
    //const HCDCodecsList::ListCodecs& rList = ResolutionDescriptor.GetCodec()->GetList();
    //HCDCodecsList::ListCodecs::const_iterator Itr(rList.begin());
    ComposedFilename += HUTClassIDDescriptor::GetInstance()->GetClassCodeCodec(codecClassKey);

    //------------------------------------
    // Raster organisation...
    if (blockType == HRFBlockType::LINE)
        ComposedFilename += "L";
    else if (blockType == HRFBlockType::STRIP)
        {
        if (ResolutionDescriptor.GetBlockType() == blockType)
            {
            if (ResolutionDescriptor.GetBlockHeight() == 1)
                ComposedFilename += "C";
            else if (!(ResolutionDescriptor.GetBlockHeight() % 16))
                ComposedFilename += "S";
            else
                ComposedFilename += "Z";
            }
        else
            {
            ComposedFilename += "C";     // default value when setting a new block type
            }
        }
    else if (blockType == HRFBlockType::TILE)
        {
        if (ResolutionDescriptor.GetBlockType() == blockType)
            {
            if (!(ResolutionDescriptor.GetBlockWidth() % 256))
                ComposedFilename += "T";
            else  if (ResolutionDescriptor.GetBlockWidth() == 64)
                ComposedFilename += "R";
            else if (!(ResolutionDescriptor.GetBlockWidth() % 32))
                ComposedFilename += "M";
            else if (!(ResolutionDescriptor.GetBlockWidth() % 16))
                ComposedFilename += "X";
            else
                ComposedFilename += "Z";
            }
        else
            {
            ComposedFilename += "T";     // default value when setting a new block type
            }
        }
    else if (blockType == HRFBlockType::IMAGE)
        ComposedFilename += "I";

    //------------------------------------
    // Resolution...
    if (pi_rpRasterFile->GetPageDescriptor(0)->CountResolutions() > 1)
        {
        // At this time we may only generate 2x multi-resolution...
        ComposedFilename += "1";
        /*
        ComposedFilename += AtpAStr("2");

        ComposedFilename += AtpAStr("3");
        */
        }
    else
        ComposedFilename += "0";


    //------------------------------------
    // iTiff Specific part
    if (pi_rpRasterFile->IsCompatibleWith(HRFiTiffFile::CLASS_ID))
        {
        //------------------------------------
        // Downsampling used
        if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::NEAREST_NEIGHBOUR)
            ComposedFilename += "N";
        else if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::AVERAGE)
            ComposedFilename += "A";
        else if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::VECTOR_AWARENESS)
            ComposedFilename += "V";
        else if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::UNKOWN)
            ComposedFilename += "U";
        else if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::ORING4)
            ComposedFilename += "4";
        else if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::NONE)
            ComposedFilename += "Z";

        //------------------------------------
        // Version..
        ComposedFilename += "2";
        }

    //------------------------------------
    // BMP Specific part.
    else if (pi_rpRasterFile->IsCompatibleWith(HRFBmpFile::CLASS_ID))
        {
        ComposedFilename += "W";
        }
    //------------------------------------
    // HMR Specific part.
    else if (pi_rpRasterFile->IsCompatibleWith(HRFHMRFile::CLASS_ID))
        {
        ComposedFilename += "1";
        }
    //------------------------------------
    // JPEG Specific part.
    else if (pi_rpRasterFile->IsCompatibleWith(HRFJpegFile::CLASS_ID))
        {
        ComposedFilename += "1";
        }
    //------------------------------------
    // Intergraph Specific part.
    else if ((pi_rpRasterFile->IsCompatibleWith(HRFIntergraphCot29File::CLASS_ID)) ||
             (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphCitFile::CLASS_ID)) ||
             (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphCotFile::CLASS_ID)) ||
             (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphRGBFile::CLASS_ID)) ||
             (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphRleFile::CLASS_ID)) ||
             (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphTG4File::CLASS_ID)))
        {
        char Buffer[4];

        itoa(ResolutionDescriptor.GetScanlineOrientation().m_ScanlineOrientation, Buffer, 10);
        ComposedFilename += Buffer;
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

    BeFileName sourceDir = ImagePPTestConfig::GetConfig().GetSourceDir();
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

    BeFileName outputRoot;
    BeTest::GetHost().GetOutputRoot(outputRoot);

    BeFileName baseRelativeOutName("ExportAllTester\\ToAllFormats\\"); //&&MM avoid hard copying the name.
    //relativeOutName += L"Images_iTIFF_1A02S142_";
    //relativeOutName += L"Images_iTIFF_TA00T1A2_";
    //relativeOutName += sourceFilename.GetFileNameWithoutExtension();
    baseRelativeOutName += sourceFilename.GetFileNameAndExtension();

    HFCPtr<HFCURL> pSourceUrl = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + sourceFilename.GetNameUtf8());

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

                    //&&MM do better. not unique for file with the same extension. ex: geotiff + tiff.
                    WString ID = s_GetProfileID(pRasterFileSource,
                                              pCurrentPixelType->GetPixelTypeClassID(),
                                              pCurrentCodec->GetCodecClassID(),
                                              pCurrentBlock->GetBlockType());

                    BeFileName outRelativeName(baseRelativeOutName);
                    outRelativeName.AppendString(L"_");
                    outRelativeName.AppendString(ID.c_str());
                    outRelativeName.AppendExtension(outExtension.c_str());

                    BeFileName outFilename(outputRoot);
                    outFilename.AppendToPath(outRelativeName.c_str());

                    //ASSERT_FALSE(outFilename.DoesPathExist()) << outFilename.c_str();

                    HFCPtr<HFCURL> pOutputUrl = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outFilename.GetNameUtf8());

                    HFCPtr<HGF2DCoordSys> pLogical = GetWorld()->GetCoordSysReference(pRasterFileSource->GetPageWorldIdentificator(0));
                    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(&GetPool(), pRasterFileSource, 0, pLogical);
                    ASSERT_TRUE(pStore != nullptr);
                    HFCPtr<HRARaster> pRaster = pStore->LoadRaster();
                    ASSERT_TRUE(pRaster != nullptr);

                    std::unique_ptr<HUTImportFromRasterExportToFile> exporter(new HUTImportFromRasterExportToFile(pRaster, *pRaster->GetEffectiveShape(), GetWorld()));
                                        
                    exporter->SelectExportFileFormat(&creator);
                    exporter->SelectExportFilename(pOutputUrl);
                    exporter->SelectPixelType(pCurrentPixelType->GetPixelTypeClassID());
                    exporter->SelectCodec(pCurrentCodec->GetCodecClassID());
                    exporter->SelectBlockType(pCurrentBlock->GetBlockType());

                    uint64_t startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
                    HFCPtr<HRFRasterFile> pFile = exporter->StartExport();
                    ASSERT_TRUE(pFile.GetPtr() != nullptr);
                    bool isTiff = pFile->IsCompatibleWith(HRFFileId_Tiff);
                    if (!isTiff && pFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
                        isTiff = reinterpret_cast<HFCPtr<HRFRasterFileExtender>&>(pFile)->GetOriginalFile()->IsCompatibleWith(HRFFileId_Tiff);
                        
                    exporter.reset();
                    pFile = nullptr;                    
                    uint64_t endTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

                    //&&MM can we do that while we export instead of reopening the file afterward?
                    //Adjust the time stamp to "9999:99:99 99:99:99"       
                    if(isTiff)
                        ASSERT_TRUE(UpdateTiffHistogramTimestamp(pOutputUrl));

                    // *** Generate export info...
                    RasterTestInfo outputInfo(outFilename);

                    static bool s_writeTestInfo = true;
                    if (s_writeTestInfo)
                        {
                        outputInfo.SetExportDuration(endTime - startTime);

                        ASSERT_TRUE(outputInfo.ComputeMD5());
                        ASSERT_TRUE(outputInfo.Store());
                        }

                    // *** validate against baseline.
                    BeFileNameCR baseLineDir = ImagePPTestConfig::GetConfig().GetBaselineDir();
                    if (!baseLineDir.empty())
                        {
                        ASSERT_TRUE(baseLineDir.DoesPathExist());

                        BeFileName baselineFilename(baseLineDir);
                        baselineFilename.AppendToPath(outRelativeName.c_str());

                        if (!baselineFilename.DoesPathExist())
                            FAIL() << "Baseline specified but file is missing : " << baselineFilename.GetNameUtf8().c_str();

                        RasterTestInfo baselineInfo(baselineFilename);

                        if (!baselineInfo.Load())
                            FAIL() << "Baseline info file is missing : " << baselineInfo.GetInfoFilename().c_str();

                        ASSERT_STREQ(baselineInfo.GetMD5().c_str(), outputInfo.GetMD5().c_str()) << "MD5 check failed";
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
                                           // Path reliative to sourceDir
                        ::testing::Combine(::testing::Values(L"Images\\iTIFF\\TA00T1A2\\TS1_32bits_None_Tile.iTIFF"),
                                           //::testing::ValuesIn(ExportAllTester::GetSourceList())
                                           ::testing::ValuesIn(ExportAllTester::GetCreatorList())
                                           ));

// #else
// 
// #pragma message("Warning: Disabling ExportAllTester because TEST_P/INSTANTIATE_TEST_CASE_P are not available")

//#endif
