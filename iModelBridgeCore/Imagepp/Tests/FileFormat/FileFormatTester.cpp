//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/FileFormat/FileFormatTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : ExportTester
//-----------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include <ImagePP/h/ImageppAPI.h>
#include <ImagePP/all/h/HRFFileFormats.h>
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>
#include <ImagePP/all/h/HUTImportFromFileExportToFile.h>

//#ifdef USE_GTEST        // TEST_P only available when using gtest.

#include <Bentley/BeDirectoryIterator.h>

#include <Imagepp/all/h/HUTClassIDDescriptor.h>


#include <Bentley/BeFileName.h>
#include <Imagepp/all/h/HPMPool.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <regex>

static uint32_t s_nbOfLoadedFiles = 0;
static uint32_t s_nbOfExportedAllOptionsFiles = 0;
static uint32_t s_nbOfFilesToBestiTiff = 0;
static uint32_t s_nbOfFilesToMostCommonOptions = 0;

USING_NAMESPACE_IMAGEPP

/*=================================================================================**//**
* Define a MACRO to display messages while testing the export.
*           PRINTF(" Hello World! \n");
*                                                   Laurent.Robert-Veillette    04/2016
+===============+===============+===============+===============+===============+======*/
namespace testing
    {
    namespace internal
        {
        enum GTestColor
            {
            COLOR_DEFAULT,
            COLOR_RED,
            COLOR_GREEN,
            COLOR_YELLOW
            };

        extern void ColoredPrintf(GTestColor color, const char* fmt, ...);
        }
    }
#define PRINTF(...)  do { testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] "); testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); } while(0)


/*=================================================================================**//**
* @bsiclass                                         Laurent.Robert-Veillette    04/2016
+===============+===============+===============+===============+===============+======*/
struct ExportTestImageppLibHost : ImagePP::ImageppLib::Host
    {
    virtual void _RegisterFileFormat() override
        {
        REGISTER_SUPPORTED_FILEFORMAT
        }

    };


/*---------------------------------------------------------------------------------**//**
* Class definition
* @bsimethod                                             Laurent.Robert-Veillette 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
class ExportTester : public ::testing::TestWithParam< std::wstring >
    {

    public:
        ExportTester()
            {
            // Required for sqlang stuff....
            BeFileName tempDir;
            BeTest::GetHost().GetTempDir(tempDir);
            BeSQLite::BeSQLiteLib::Initialize(tempDir);

            //Initialize host
            ImagePP::ImageppLib::Initialize(*new ExportTestImageppLibHost());
            }
        ~ExportTester()
            {
            ImagePP::ImageppLib::GetHost().Terminate(true);
            };
    };

///*---------------------------------------------------------------------------------**//**
//* Return a vector with all the paths to the rasters or directories.
//* @bsimethod                                             Laurent.Robert-Veillette 04/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
//static vector<std::wstring> s_GetDirectoryVector(const Utf8String& pSourceDir)
//    {
//    vector<std::wstring> directoryList;
//
//    BeFileName sourcePath(pSourceDir);
//    BeFileName actualPath;
//    bool isDir;
//    for (BeDirectoryIterator dir(sourcePath); dir.GetCurrentEntry(actualPath, isDir, true) == SUCCESS; dir.ToNext())
//        {
//        if (isDir)
//            continue;//What do we do?? Skip?
//        else
//            directoryList.push_back(actualPath.GetName());
//        }
//
//    return directoryList;
//    }

/*---------------------------------------------------------------------------------**//**
* Return a vector with all the paths to the rasters or directories.
* @bsimethod                                             Laurent.Robert-Veillette 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static vector<std::wstring> s_GetDirectoryVector(const WString& pSourceDir)
    {
    vector<std::wstring> directoryList;

    BeFileListIterator sourcePathItr(pSourceDir, true);
    BeFileName actualName;
    while (sourcePathItr.GetNextFileName(actualName) == SUCCESS)
        {
        if (actualName.IsDirectory())
            continue;
        else
            {            
            std::regex filter("(.*)(NITF\\\\PasSupportees)(.*)|(.*)(ErdasImg\\\\ImagesInvalides)(.*)|(.*)(iTIFF\\\\xFileNotSupported)(.*)");

            //exclusion of the files not supported
            if (std::regex_match(actualName.GetNameUtf8().c_str(), filter))
                continue;

            directoryList.push_back(actualName.GetName());
            }           
        }

    return directoryList;
    }

static const size_t s_nbTotalOfFiles = s_GetDirectoryVector(L"C:\\Users\\Laurent.Robert-Veill\\Desktop\\Images\\*").size();

///*---------------------------------------------------------------------------------**//**
//* Format the name of the file with the specific options of the export
//* @bsimethod                                         Laurent.Robert-Veillette     04/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
//static BeFileName s_GetProfileID(const HFCPtr<HRFRasterFile>&   pi_rpRasterFile,
//                                 HCLASS_ID                      pixelTypeClassKey,
//                                 HCLASS_ID                      codecClassKey,
//                                 HRFBlockType                   blockType)
//    {
//    Utf8String ComposedFilename;
//
//    HRFResolutionDescriptor ResolutionDescriptor(*(pi_rpRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)));
//
//    //------------------------------------
//    // Color space (Pixel Type)
//    ComposedFilename += HUTClassIDDescriptor::GetInstance()->GetClassCodePixelType(pixelTypeClassKey);
//
//    //------------------------------------
//    // Compression Codec
//    //const HCDCodecsList::ListCodecs& rList = ResolutionDescriptor.GetCodec()->GetList();
//    //HCDCodecsList::ListCodecs::const_iterator Itr(rList.begin());
//    ComposedFilename += HUTClassIDDescriptor::GetInstance()->GetClassCodeCodec(codecClassKey);
//
//    //------------------------------------
//    // Raster organisation...
//    if (blockType == HRFBlockType::LINE)
//        ComposedFilename += "L";
//    else if (blockType == HRFBlockType::STRIP)
//        {
//        if (ResolutionDescriptor.GetBlockType() == blockType)
//            {
//            if (ResolutionDescriptor.GetBlockHeight() == 1)
//                ComposedFilename += "C";
//            else if (!(ResolutionDescriptor.GetBlockHeight() % 16))
//                ComposedFilename += "S";
//            else
//                ComposedFilename += "Z";
//            }
//        else
//            {
//            ComposedFilename += "C";     // default value when setting a new block type
//            }
//        }
//    else if (blockType == HRFBlockType::TILE)
//        {
//        if (ResolutionDescriptor.GetBlockType() == blockType)
//            {
//            if (!(ResolutionDescriptor.GetBlockWidth() % 256))
//                ComposedFilename += "T";
//            else  if (ResolutionDescriptor.GetBlockWidth() == 64)
//                ComposedFilename += "R";
//            else if (!(ResolutionDescriptor.GetBlockWidth() % 32))
//                ComposedFilename += "M";
//            else if (!(ResolutionDescriptor.GetBlockWidth() % 16))
//                ComposedFilename += "X";
//            else
//                ComposedFilename += "Z";
//            }
//        else
//            {
//            ComposedFilename += "T";     // default value when setting a new block type
//            }
//        }
//    else if (blockType == HRFBlockType::IMAGE)
//        ComposedFilename += "I";
//
//    //------------------------------------
//    // Resolution...
//    if (pi_rpRasterFile->GetPageDescriptor(0)->CountResolutions() > 1)
//        {
//        // At this time we may only generate 2x multi-resolution...
//        ComposedFilename += "1";
//        /*
//        ComposedFilename += AtpAStr("2");
//
//        ComposedFilename += AtpAStr("3");
//        */
//        }
//    else
//        ComposedFilename += "0";
//
//
//    //------------------------------------
//    // iTiff Specific part
//    if (pi_rpRasterFile->IsCompatibleWith(HRFiTiffFile::CLASS_ID))
//        {
//        //------------------------------------
//        // Downsampling used
//        if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::NEAREST_NEIGHBOUR)
//            ComposedFilename += "N";
//        else if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::AVERAGE)
//            ComposedFilename += "A";
//        else if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::VECTOR_AWARENESS)
//            ComposedFilename += "V";
//        else if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::UNKOWN)
//            ComposedFilename += "U";
//        else if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::ORING4)
//            ComposedFilename += "4";
//        else if (ResolutionDescriptor.GetDownSamplingMethod() == HRFDownSamplingMethod::NONE)
//            ComposedFilename += "Z";
//
//        //------------------------------------
//        // Version..
//        ComposedFilename += "2";
//        }
//
//    //------------------------------------
//    // BMP Specific part.
//    else if (pi_rpRasterFile->IsCompatibleWith(HRFBmpFile::CLASS_ID))
//        {
//        ComposedFilename += "W";
//        }
//    //------------------------------------
//    // HMR Specific part.
//    else if (pi_rpRasterFile->IsCompatibleWith(HRFHMRFile::CLASS_ID))
//        {
//        ComposedFilename += "1";
//        }
//    //------------------------------------
//    // JPEG Specific part.
//    else if (pi_rpRasterFile->IsCompatibleWith(HRFJpegFile::CLASS_ID))
//        {
//        ComposedFilename += "1";
//        }
//    //------------------------------------
//    // Intergraph Specific part.
//    else if ((pi_rpRasterFile->IsCompatibleWith(HRFIntergraphCot29File::CLASS_ID)) ||
//             (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphCitFile::CLASS_ID)) ||
//             (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphCotFile::CLASS_ID)) ||
//             (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphRGBFile::CLASS_ID)) ||
//             (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphRleFile::CLASS_ID)) ||
//             (pi_rpRasterFile->IsCompatibleWith(HRFIntergraphTG4File::CLASS_ID)))
//        {
//        char Buffer[4];
//
//        itoa(ResolutionDescriptor.GetScanlineOrientation().m_ScanlineOrientation, Buffer, 10);
//        ComposedFilename += Buffer;
//        }
//
//    return BeFileName(ComposedFilename);
//    }

//:>--------------------------------------------------------------------------------------+
//
// Test the load of the rasters. Reading test.
//                                                Laurent.Robert-Veillette            04/2016
//:>+--------------------------------------------------------------------------------------
TEST_P(ExportTester, LoadingRasters)
    {
    ++s_nbOfLoadedFiles;
    PRINTF("Trying to read file %u out of %u\n", s_nbOfLoadedFiles, s_nbTotalOfFiles);

    //Test that the source paths is not empty
    ASSERT_FALSE(GetParam().empty());

    HFCPtr<HGF2DWorldCluster> worldCluster = new HGFHMRStdWorldCluster();
    HPMPool pPool(64 * 1024, nullptr);

    try
        {
        HFCPtr<HFCURL> UrlSource = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GetParam().c_str()));
        HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
        if (pSrcCreator == nullptr)
            return;

        HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(UrlSource, HFC_READ_ONLY);
        //Verify that the Rasterfile created is valid
        ASSERT_FALSE(nullptr == pRasterFileSource);

        HFCPtr<HGF2DCoordSys> pLogical = worldCluster->GetCoordSysReference(pRasterFileSource->GetPageWorldIdentificator(0));
        HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(&pPool, pRasterFileSource, 0, pLogical);
        //HFCPtr<HPSObjectStore> pStore = new HPSObjectStore(&pPool, UrlSource, worldCluster);
        ASSERT_TRUE(nullptr != pStore);


        HFCPtr<HRARaster> pRaster = pStore->LoadRaster();
        ASSERT_FALSE(pRaster == nullptr);
        //ASSERT_NO_THROW(pStore->LoadRaster());
        }
    catch (HFCException& excep)
        {
        //&&LRV filter what are common error and what are not.
        WString message(excep.GetExceptionMessage().c_str(), BentleyCharEncoding::Utf8);
        FAIL() << message.c_str();
        }
    }

////:>--------------------------------------------------------------------------------------+
//// Test the export from a raster file to iTiff file with all the possible combinations 
//// of the following options:
////      All the possible block types
////      24 bits Pixel Type associated with JPEG Codec
////      32 bits Pixel Type assicated with Deflate Codec
////                                                Laurent.Robert-Veillette            04/2016
////:>+--------------------------------------------------------------------------------------
//TEST_P(ExportTester, ExportToiTiffMostCommonOptions)
//    {
//    ++s_nbOfFilesToMostCommonOptions;
//    PRINTF("Trying to export file to iTiff with most common options %u out of %u\n", s_nbOfFilesToMostCommonOptions, s_nbTotalOfFiles);
//    //Test that the source paths is not empty
//    ASSERT_FALSE(GetParam().empty());
//
//    HRFiTiffCreator* creator = HRFiTiffCreator::GetInstance();
//    ASSERT_TRUE(creator->GetSupportedAccessMode().m_HasCreateAccess);
//
//    HFCPtr<HFCURL> UrlSource = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GetParam().c_str()));
//    HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
//    if (pSrcCreator == nullptr)
//        return;
//
//    HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
//    //Verify that the Rasterfile created is valid
//    ASSERT_FALSE(nullptr == pRasterFileSource);
//
//    HFCPtr<HGF2DWorldCluster> worldCluster = new HGFHMRStdWorldCluster();
//
//    BeFileName outputFilePath;
//    BeTest::GetHost().GetOutputRoot(outputFilePath);
//    BeFileName::CreateNewDirectory(outputFilePath.c_str());
//
//    const HFCPtr<HRFRasterFileCapabilities> PixelCapabilities = creator->GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID, HFC_CREATE_ONLY);   
//    ASSERT_FALSE(nullptr == PixelCapabilities);
//    const HFCPtr<HRFRasterFileCapabilities> CodecCapabilities = creator->GetCapabilities()->GetCapabilitiesOfType(HRFCodecCapability::CLASS_ID, HFC_CREATE_ONLY);
//    ASSERT_FALSE(nullptr == CodecCapabilities);
//    const HFCPtr<HRFRasterFileCapabilities> BlockTypeCapabilities = creator->GetCapabilities()->GetCapabilitiesOfType(HRFBlockCapability::CLASS_ID, HFC_CREATE_ONLY);
//    ASSERT_FALSE(nullptr == BlockTypeCapabilities);
//
//    // Getting pixel type capabilities
//    HRFPixelTypeCapability pixelType24Bits(HFC_CREATE_ONLY, HRPPixelTypeV24R8G8B8::CLASS_ID, CodecCapabilities);
//
//    // Getting Codec type capabilities    
//    HRFCodecCapability codecJPEG(HFC_CREATE_ONLY, HCDCodecId_JPEG, BlockTypeCapabilities);
//    
//
//    for (uint32_t blockIndex(0); blockIndex < BlockTypeCapabilities->CountCapabilities(); ++blockIndex)
//        {
//        const HFCPtr<HRFBlockCapability> pCurrentBlock = (const HFCPtr<HRFBlockCapability>&) BlockTypeCapabilities->GetCapability(blockIndex);
//        ASSERT_FALSE(nullptr == pCurrentBlock);
//
//        // If it is not possible to write a new file using the current block type, we skip it too
//        if (!pCurrentBlock->GetAccessMode().m_HasCreateAccess)
//            continue;
//
//        // Building the full path where the image will be saved                
//        BeFileName ID = s_GetProfileID(pRasterFileSource,
//                                       pixelType24Bits.GetPixelTypeClassID(),
//                                       codecJPEG.GetCodecClassID(),
//                                       pCurrentBlock->GetBlockType());
//
//        BeFileName SourceFile(GetParam().c_str());
//        BeFileName outfilename(outputFilePath);
//        outfilename.AppendToPath(SourceFile.GetFileNameWithoutExtension().c_str());
//        outfilename.append(L"_");
//        outfilename.append(ID);
//
//        HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outfilename.GetNameUtf8());
//
//        HUTImportFromFileExportToFile exporter(worldCluster);
//
//        exporter.SetImportRasterFile(pRasterFileSource);
//        exporter.SelectExportFileFormat(creator);
//        exporter.SelectExportFilename(pURL);
//        exporter.SelectPixelType(pixelType24Bits.GetPixelTypeClassID());
//        exporter.SelectCodec(codecJPEG.GetCodecClassID());
//        exporter.SelectBlockType(pCurrentBlock->GetBlockType());
//
//        try
//            {
//            // Exporting
//            HFCPtr<HRFRasterFile> pFile = exporter.StartExport();
//            ASSERT_TRUE(pFile.GetPtr() != nullptr);
//            //ASSERT_NO_THROW(exporter.StartExport());
//            }
//        catch (...)
//            {
//            //&&MM filter what are common error and what are not.
//            FAIL();
//            }
//        }
//    }
//
//
////:>--------------------------------------------------------------------------------------+
////
//// Test the export from a raster file to iTiff file with the best possible options.
////                                                Laurent.Robert-Veillette            04/2016
////:>+--------------------------------------------------------------------------------------
//TEST_P(ExportTester, ExportToiTiffBestOptions)
//    {
//    ++s_nbOfFilesToBestiTiff;
//    PRINTF("Trying to export file to best iTiff %u out of %u\n", s_nbOfFilesToBestiTiff, s_nbTotalOfFiles);
//    //Test that the source paths is not empty
//    ASSERT_FALSE(GetParam().empty());
//
//    // ExportToBestiTiff
//    HFCPtr<HGF2DWorldCluster> worldCluster = new HGFHMRStdWorldCluster();
//    HPMPool pPool(64 * 1024, nullptr);
//
//    HFCPtr<HFCURL> UrlSource = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GetParam().c_str()));
//    HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
//    if (pSrcCreator == nullptr)
//        return;
//
//    HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
//    //Verify that the Rasterfile created is valid
//    ASSERT_FALSE(nullptr == pRasterFileSource);
//
//
//    HFCPtr<HGF2DCoordSys> pLogical = worldCluster->GetCoordSysReference(pRasterFileSource->GetPageWorldIdentificator(0));
//    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(&pPool, pRasterFileSource, 0, pLogical);
//    ASSERT_TRUE(pStore != nullptr);
//    HFCPtr<HRARaster> pRaster = pStore->LoadRaster();
//    ASSERT_TRUE(pRaster != nullptr);
//
//    HUTImportFromRasterExportToFile exporter(pRaster, *pRaster->GetEffectiveShape(), worldCluster);
//    exporter.SelectExportFileFormat((HRFRasterFileCreator*) HRFiTiffCreator::GetInstance());
//
//    // Use BestMatchSelectedValues for exportation
//    ASSERT_NO_THROW(exporter.BestMatchSelectedValues());
//
//    // Export image to specified output path
//    BeFileName inputFileName(GetParam().c_str());
//    BeFileName outputFilePath;
//    BeTest::GetHost().GetOutputRoot(outputFilePath);
//    HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outputFilePath.GetNameUtf8() + Utf8String(inputFileName.GetFileNameWithoutExtension()));
//    exporter.SelectExportFilename(pURL);
//    try
//        {
//        HFCPtr<HRFRasterFile> pFile = exporter.StartExport();
//        ASSERT_TRUE(pFile.GetPtr() != nullptr);
//        }
//    catch (...)
//        {
//        //&&LRV filter what are common error and what are not.
//        FAIL();
//        }
//    }


//:>--------------------------------------------------------------------------------------+
//
// Test the export from a raster file to iTiff file. Iterates on every possible options of 
// pixel type, codec and block type.
//                                             Laurent.Robert-Veillette            04/2016
//:>+--------------------------------------------------------------------------------------
//TEST_P(ExportTester, FromFileToiTiffExportAllOptions)
//    {
//    ++s_nbOfExportedAllOptionsFiles;
//    PRINTF("Trying to export to all options file %u out of %zu\n", s_nbOfExportedAllOptionsFiles, s_nbTotalOfFiles);
//    ASSERT_FALSE(GetParam().empty());   // Test that the source paths is not empty
//
//    HRFiTiffCreator* creator = HRFiTiffCreator::GetInstance();
//    ASSERT_TRUE(creator->GetSupportedAccessMode().m_HasCreateAccess);
//
//    HFCPtr<HFCURL> UrlSource = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GetParam().c_str()));
//    HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
//    //&&MM find a way to ignore some file extension? ex: world file?? if it throws that generates a failure
//    // even if we were able to ignore exceptions we might be ignoring a real exception
//    if (pSrcCreator == nullptr)
//        return;
//
//    //&&MM reopen the source for each iteration? Need to determine what we really want to test.
//    //      if we want to test read of source why bother exporting to all itiff for every file aren't we loosing our time?
//    HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
//    
//    //Verify that the Rasterfile created is valid
//    ASSERT_FALSE(nullptr == pRasterFileSource);    
//
//    // Getting pixel type capabilities
//    const HFCPtr<HRFRasterFileCapabilities> capabilities = creator->GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID, HFC_CREATE_ONLY);
//
//    HFCPtr<HGF2DWorldCluster> worldCluster = new HGFHMRStdWorldCluster();
//
//    BeFileName outputFilePath;
//    BeTest::GetHost().GetOutputRoot(outputFilePath);
//    BeFileName::CreateNewDirectory(outputFilePath.c_str());
//
//    //Run the export on every possible pixel types, codecs and block types
//    for (uint32_t ptIndex(0); ptIndex < capabilities->CountCapabilities(); ++ptIndex)
//        {
//        const HFCPtr<HRFPixelTypeCapability> pCurrentPixelType = (const HFCPtr<HRFPixelTypeCapability>&)(capabilities->GetCapability(ptIndex));
//
//        // If it is not possible to write a new file using the current pixel type, we skip it
//        if (!pCurrentPixelType->GetAccessMode().m_HasCreateAccess)
//            continue;
//
//        // Getting codec capabilities for every pixel type
//        const HFCPtr<HRFRasterFileCapabilities> codecCaps = pCurrentPixelType->GetCodecCapabilities();
//        for (uint32_t codecIndex(0); codecIndex < codecCaps->CountCapabilities(); ++codecIndex)
//            {
//            const HFCPtr<HRFCodecCapability> pCurrentCodec = (const HFCPtr<HRFCodecCapability>&) codecCaps->GetCapability(codecIndex);
//
//            // If it is not possible to write a new file using the current codec, we skip it too
//            if (!pCurrentCodec->GetAccessMode().m_HasCreateAccess)
//                continue;
//
//            const HFCPtr<HRFRasterFileCapabilities> blockCaps = pCurrentCodec->GetBlockTypeCapabilities();
//            for (uint32_t blockIndex(0); blockIndex < blockCaps->CountCapabilities(); ++blockIndex)
//                {
//                const HFCPtr<HRFBlockCapability> pCurrentBlock = (const HFCPtr<HRFBlockCapability>&) blockCaps->GetCapability(blockIndex);
//
//                // If it is not possible to write a new file using the current block type, we skip it too
//                if (!pCurrentBlock->GetAccessMode().m_HasCreateAccess)
//                    continue;
//
//                // Building the full path where the image will be saved                
//                BeFileName ID = s_GetProfileID(pRasterFileSource,
//                                             pCurrentPixelType->GetPixelTypeClassID(),
//                                             pCurrentCodec->GetCodecClassID(),
//                                             pCurrentBlock->GetBlockType());
//                
//                
//
//                BeFileName SourceFile(GetParam().c_str());
//                BeFileName outfilename(outputFilePath);
//                outfilename.AppendToPath(SourceFile.GetFileNameWithoutExtension().c_str());
//                outfilename.append(L"_");
//                outfilename.append(ID);
//                //newName.AppendUtf8(creator->GetDefaultExtension().c_str());
//                //outfilename.append(newName);
//
//                //outputFilePath.AppendToPath(filename.c_str());
//                HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outfilename.GetNameUtf8());
//                                  
//                std::unique_ptr<HUTImportFromFileExportToFile> exporter(new HUTImportFromFileExportToFile(worldCluster));
//
//                exporter->SetImportRasterFile(pRasterFileSource);
//                exporter->SelectExportFileFormat(creator);
//                exporter->SelectExportFilename(pURL);
//                exporter->SelectPixelType(pCurrentPixelType->GetPixelTypeClassID());
//                exporter->SelectCodec(pCurrentCodec->GetCodecClassID());
//                exporter->SelectBlockType(pCurrentBlock->GetBlockType());
//
//                try
//                    {
//                    // Exporting
//                    HFCPtr<HRFRasterFile> pFile = exporter.StartExport();
//                    ASSERT_TRUE(pFile.GetPtr() != nullptr);
//                    //ASSERT_NO_THROW(exporter.StartExport());
//                    }                
//                catch (...)
//                    {
//                    //&&MM filter what are common error and what are not.
//                    FAIL();
//                    }                
//                }
//            }
//        }
//    }

//:>--------------------------------------------------------------------------------------+
//
//Instantiate (run) all the TEST_P of ExportTester with the files found in the argument path
//
//:>+--------------------------------------------------------------------------------------
INSTANTIATE_TEST_CASE_P(AllRastersInDirectory, ExportTester,
                        ::testing::ValuesIn(s_GetDirectoryVector(L"C:\\Users\\Laurent.Robert-Veill\\Desktop\\Images\\*")));



// #else
// 
// #pragma message("Warning: Disabling ExportTester because TEST_P/INSTANTIATE_TEST_CASE_P are not available")

//#endif
