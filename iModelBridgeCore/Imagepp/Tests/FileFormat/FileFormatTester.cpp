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
#include <ImagePP/all/h/HUTImportFromRasterExportToFile.h>

//#ifdef USE_GTEST        // TEST_P only available when using gtest.

#include <Imagepp/all/h/HUTClassIDDescriptor.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeFileName.h>
#include <Imagepp/all/h/HPMPool.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HCDCodecZlib.h>

USING_NAMESPACE_IMAGEPP

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
            BeFileName::CreateNewDirectory(tempDir.c_str());
            BeSQLite::BeSQLiteLib::Initialize(tempDir);

            //Initialize host
            ImagePP::ImageppLib::Initialize(*new ExportTestImageppLibHost());

            //&&MM generate a failure if it fails. we need geocoord to read geotag.
            BeFileName geoCoordData;
            BeTest::GetHost().GetDgnPlatformAssetsDirectory(geoCoordData);
            geoCoordData.AppendToPath(L"DgnGeoCoord");
            GeoCoordinates::BaseGCS::Initialize(geoCoordData.c_str());
            }
        ~ExportTester()
            {
            ImagePP::ImageppLib::GetHost().Terminate(true);
            };
    };

/*---------------------------------------------------------------------------------**//**
* Return a vector with all the paths to the rasters or directories.
* @bsimethod                                             Laurent.Robert-Veillette 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static vector<std::wstring> s_GetFileNameVector()
    {
    vector<std::wstring> directoryList;
    bvector<BeFileName> fileList;

    //&&MM that path should come from the asset directory(symlink during build) or
    // from a config file ?
    //&&MM pss and dem are missing from the the test case.
//    BeFileName sourcePath("D:\\Dataset\\Images_Files\\_forATPs\\Images");
    BeFileName sourcePath("D:\\Dataset\\Images_Files\\_forATPs\\DEM");

    const WString glob = L"*";

    BeDirectoryIterator::WalkDirsAndMatch(fileList, sourcePath, glob.c_str(), true);

    // Scan the fileList and skipping not supported rasters and folders
    for (auto& actualName : fileList)
        {
        if (actualName.IsDirectory() ||
            actualName.ContainsI(L"thumb.db")                                     // Ignore thumnail windows files.
            )
            continue;

        directoryList.push_back(actualName.GetName());
        }

    return directoryList;
    }


/*---------------------------------------------------------------------------------**//**
* Format the name of the file following the block type given
* returns a WString that contains the first letter of the block type.
* @bsimethod                                         Laurent.Robert-Veillette     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
//static WString s_GetBlockNumberToChar(const HRFBlockType& blockType)
//    {
//    switch (blockType.m_BlockType)
//        {
//            case HRFBlockType::LINE:
//                return L"L";
//                break;
//            case HRFBlockType::STRIP:
//                return L"S";
//                break;
//            case HRFBlockType::TILE:
//                return L"T";
//                break;
//            case HRFBlockType::IMAGE:
//                return L"I";
//                break;
//            case HRFBlockType::AUTO_DETECT:
//                return L"A";
//                break;
//            default:
//                BeAssert(!"Should not get here");
//                return L"";
//        }
//    }
//
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

/*---------------------------------------------------------------------------------**//**
@description  Depending on the image format of the input file, it makes required
*               modifications for known issues with certain formats.
*
* see ..\HugeDataTest\src\Apps\ImagePP\Wrapper\TestFile.cpp
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_UpdateTiffHistogramTimestamp(HFCPtr<HFCURL> const& pUrl)
    {
    if (!pUrl->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    HAutoPtr<HTIFFFile> pTiffFile(new HTIFFFile(static_cast<HFCURLFile*>(pUrl.GetPtr())->GetAbsoluteFileName(), HFC_READ_WRITE));

    HTIFFError* pErr = NULL;
    if (pErr != NULL && pErr->IsFatal())
        return false;

    AString value("9999:99:99 99:99:99");

    //Standard Directories
    unsigned int standardDirectoryNb = pTiffFile->NumberOfDirectory(HTIFFFile::STANDARD);

    bool NeedSave = false;

    for (unsigned int i = 0; standardDirectoryNb > i; ++i)
        {
        pTiffFile->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, i));

        if (pTiffFile->TagIsPresent(ImagePP::DATETIME))
            {
            pTiffFile->SetFieldA(ImagePP::DATETIME, value.c_str());
            NeedSave = true;
            }

        if (pTiffFile->TagIsPresent(ImagePP::HMR_HISTOGRAMDATETIME))
            {
            pTiffFile->SetFieldA(ImagePP::HMR_HISTOGRAMDATETIME, value.c_str());
            NeedSave = true;
            }
        }

    //HMR directories
    unsigned int hmrDirectoryNb = pTiffFile->NumberOfDirectory(HTIFFFile::HMR);

    for (unsigned int i = 0; hmrDirectoryNb > i; ++i)
        {
        pTiffFile->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, i));

        if (pTiffFile->TagIsPresent(ImagePP::DATETIME))
            {
            pTiffFile->SetFieldA(ImagePP::DATETIME, value.c_str());
            NeedSave = true;
            }

        if (pTiffFile->TagIsPresent(ImagePP::HMR_HISTOGRAMDATETIME))
            {
            pTiffFile->SetFieldA(ImagePP::HMR_HISTOGRAMDATETIME, value.c_str());
            NeedSave = true;
            }
        }

    if (NeedSave)
        pTiffFile->Save();

    pTiffFile = 0;

    return true;
    }

//:>--------------------------------------------------------------------------------------+
//
// Test the export from a raster file to iTiff file with the best possible options.
//                                                Laurent.Robert-Veillette            04/2016
//:>+--------------------------------------------------------------------------------------
TEST_P(ExportTester, ExportToiTiffBestOptions)
    {
    //Test that the source paths is not empty
    ASSERT_FALSE(GetParam().empty());
    ASSERT_TRUE(GeoCoordinates::BaseGCS::IsLibraryInitialized());

    // ExportToBestiTiff
    HFCPtr<HGF2DWorldCluster> worldCluster = new HGFHMRStdWorldCluster();
    HPMPool pPool(200 * 1024, nullptr);

    HFCPtr<HFCURL> UrlSource = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GetParam().c_str()));

    try
        {
        //N.B. We set the 4th parameter ScanCreatorIfNotFound = true because some files do not have the good extension. 
        //Hence we cannot rely only on the extension to find the appropriate creator.
        //>>> &&MM turning it OFF for now. It is slow. fill fix file extention once we have validated the baseline.
        bool scanCreator = false;
        HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE, 0, scanCreator, false);
        if (pSrcCreator == nullptr)
            return;

        HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
        //Verify that the Rasterfile created is valid
        ASSERT_FALSE(nullptr == pRasterFileSource);

#if 1   // Old base line was created with file to file and that will copy extra meta data information. we need them to compare with old baseline.
        std::unique_ptr<HUTImportFromFileExportToFile> exporter(new HUTImportFromFileExportToFile(worldCluster));
        exporter->SelectExportFileFormat(HRFiTiffCreator::GetInstance());
        exporter->SetImportRasterFile(pRasterFileSource);

#else        
        HFCPtr<HGF2DCoordSys> pLogical = worldCluster->GetCoordSysReference(pRasterFileSource->GetPageWorldIdentificator(0));
        HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(&pPool, pRasterFileSource, 0, pLogical);
        ASSERT_TRUE(pStore != nullptr);
        HFCPtr<HRARaster> pRaster = pStore->LoadRaster();
        ASSERT_TRUE(pRaster != nullptr);

        std::unique_ptr<HUTImportFromRasterExportToFile> exporter(new HUTImportFromRasterExportToFile(pRaster, *pRaster->GetEffectiveShape(), worldCluster));
        exporter->SelectExportFileFormat(HRFiTiffCreator::GetInstance());

#endif

        // Use BestMatchSelectedValues for exportation
        ASSERT_NO_THROW(exporter->BestMatchSelectedValues());

        //&&MM best match should use fax4 for binary and not RLE. make sure it is strip also.

        //Construct the output file path to fit the one from the old baseline
        /// >> &&MM chage to whatever once we have validated the baseline.
        //     and make a unique directory per specific test to avoid clash.
        //     and do what we can during test setup.
        BeFileName outputFilePath;
        BeTest::GetHost().GetOutputRoot(outputFilePath);
        auto positionStart = GetParam().find(L"Images\\");
        if (positionStart == WString::npos)                     // Try with DEM if Images not there.
            positionStart = GetParam().find(L"DEM\\");
        auto positionEnd = GetParam().find(L"\\", positionStart + 7);
        WString folderNameToAppend(GetParam().substr(positionStart, positionEnd - positionStart).c_str());
        outputFilePath.AppendToPath(folderNameToAppend.c_str());
        outputFilePath.AppendToPath(L"Default");

        //Create the ouput folder if not already created
        if (!outputFilePath.DoesPathExist())
            {
            BeFileNameStatus status = BeFileName::CreateNewDirectory(outputFilePath.GetName());
            ASSERT_TRUE(BeFileNameStatus::Success == status);
            }

        auto pos = GetParam().find(L"Images");
        if (pos == WString::npos)                     // Try with DEM if Images not there.
            pos = GetParam().find(L"DEM");
        WString newNameFile(GetParam().substr(pos).c_str());
        newNameFile.ReplaceAll(L"\\", L"_");
        outputFilePath.AppendToPath(newNameFile.c_str());

        HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outputFilePath.GetNameUtf8() + ".itiff");
        exporter->SelectExportFilename(pURL);

        HFCPtr<HRFRasterFile> pFile = exporter->StartExport();
        ASSERT_TRUE(pFile.GetPtr() != nullptr);  

//         HFCPtr<HPMGenericAttribute> pTag;  pTag = new HRFAttributeDateTime(pSystem);
//         HRFAttributeDateTime
//         HRFAttributeDateTime

        //&&MM what kind of post validation can we do?

        //Close the RasterFile in order to be able to open it again after
        pFile = nullptr;

        //&&MM can we do that while we export instead of reopening the file afterward?
        //Adjust the time stamp to "9999:99:99 99:99:99"       
        ASSERT_TRUE(s_UpdateTiffHistogramTimestamp(pURL));
        }
    catch (HFCException& e)
        {
        FAIL() << e.GetExceptionMessage().c_str();
        }
    }

//:>--------------------------------------------------------------------------------------+
//
// Test the load of the rasters. Reading test.
//                                                Laurent.Robert-Veillette            04/2016
//:>+--------------------------------------------------------------------------------------
//TEST_P(ExportTester, LoadingRasters)
//    {
//    //Test that the source paths is not empty
//    ASSERT_FALSE(GetParam().empty());
//
//    HFCPtr<HGF2DWorldCluster> worldCluster = new HGFHMRStdWorldCluster();
//    HPMPool pPool(64 * 1024, nullptr);
//
//    try
//        {
//        HFCPtr<HFCURL> UrlSource = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GetParam().c_str()));
//
//        //N.B. We set the 4th parameter ScanCreatorIfNotFound = true because some files do not have the good extension. 
//        //Hence we cannot rely only on the extension to find the appropriate creator.
//        HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE, 0, true, false);
//        if (pSrcCreator == nullptr)
//            return;
//
//        HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(UrlSource, HFC_READ_ONLY);
//        //Verify that the Rasterfile created is valid
//        ASSERT_FALSE(nullptr == pRasterFileSource);
//
//        HFCPtr<HGF2DCoordSys> pLogical = worldCluster->GetCoordSysReference(pRasterFileSource->GetPageWorldIdentificator(0));
//        HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(&pPool, pRasterFileSource, 0, pLogical);
//        ASSERT_TRUE(nullptr != pStore);
//
//
//        HFCPtr<HRARaster> pRaster = pStore->LoadRaster();
//        ASSERT_FALSE(pRaster == nullptr);
//        //ASSERT_NO_THROW(pStore->LoadRaster());
//        }
//    catch (...)
//        {
//        FAIL();
//        }
//    }
//
////:>--------------------------------------------------------------------------------------+
//// Test the export from a raster file to iTiff file with :
////      All the possible block types
////      24 bits Pixel Type associated with JPEG Codec (see HCDCodecIJG)
////                                                Laurent.Robert-Veillette            04/2016
////:>+--------------------------------------------------------------------------------------
//TEST_P(ExportTester, ExportToiTiff24BitsJPEG)
//    {
//    //Test that the source paths is not empty
//    ASSERT_FALSE(GetParam().empty());
//
//    HRFiTiffCreator* creator = HRFiTiffCreator::GetInstance();
//    ASSERT_TRUE(creator->GetSupportedAccessMode().m_HasCreateAccess);
//
//    HFCPtr<HFCURL> UrlSource = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GetParam().c_str()));
//
//    try
//        {
//        //N.B. We set the 4th parameter ScanCreatorIfNotFound = true because some files do not have the good extension. 
//        //Hence we cannot rely only on the extension to find the appropriate creator.
//        HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE, 0, true, false);
//        if (pSrcCreator == nullptr)
//            return;
//
//        HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
//        //Verify that the Rasterfile created is valid
//        ASSERT_FALSE(nullptr == pRasterFileSource);
//
//        HFCPtr<HGF2DWorldCluster> worldCluster = new HGFHMRStdWorldCluster();
//
//        //Get all pixel types available for this creator
//        const HFCPtr<HRFRasterFileCapabilities> PixelCapabilities = creator->GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID, HFC_CREATE_ONLY);
//        ASSERT_FALSE(nullptr == PixelCapabilities);
//
//        //Create the pixel type 24 bits
//        HFCPtr<HRFPixelTypeCapability> pPixel24Bits = new HRFPixelTypeCapability(HFC_CREATE_ONLY, HRPPixelTypeV24R8G8B8::CLASS_ID, new HRFRasterFileCapabilities());
//        pPixel24Bits = static_cast<HRFPixelTypeCapability*>(PixelCapabilities->GetCapabilityOfType(((HFCPtr<HRFCapability>)pPixel24Bits)).GetPtr());
//        ASSERT_FALSE(nullptr == pPixel24Bits);
//
//        //Create the codec IJG_JPEG
//        const HFCPtr<HRFRasterFileCapabilities> CodecCapabilities = pPixel24Bits->GetCodecCapabilities();
//        ASSERT_FALSE(nullptr == CodecCapabilities);
//
//        HFCPtr<HRFCodecCapability> pCodecJpeg = new HRFCodecCapability(HFC_CREATE_ONLY, HCDCodecIJG::CLASS_ID, new HRFRasterFileCapabilities());
//        pCodecJpeg = static_cast<HRFCodecCapability*>(CodecCapabilities->GetCapabilityOfType(((HFCPtr<HRFCapability>)pCodecJpeg)).GetPtr());
//        ASSERT_FALSE(nullptr == pCodecJpeg);
//
//        //Iterate on all block types for 24bits - JPEG
//        BeFileName outputFilePath;
//        std::unique_ptr<HUTImportFromFileExportToFile> exporter(new HUTImportFromFileExportToFile(worldCluster));
//
//        exporter->SetImportRasterFile(pRasterFileSource);
//        exporter->SelectExportFileFormat(creator);
//
//        const HFCPtr<HRFRasterFileCapabilities> BlockCapabilities = pCodecJpeg->GetBlockTypeCapabilities();
//        ASSERT_FALSE(nullptr == BlockCapabilities);
//
//        for (uint32_t blockIndex(0); blockIndex < BlockCapabilities->CountCapabilities(); ++blockIndex)
//            {
//            const HFCPtr<HRFBlockCapability> pCurrentBlock = (const HFCPtr<HRFBlockCapability>&) BlockCapabilities->GetCapability(blockIndex);
//            ASSERT_FALSE(nullptr == pCurrentBlock);
//
//            // If it is not possible to write a new file using the current block type, we skip it
//            if (!pCurrentBlock->GetAccessMode().m_HasCreateAccess)
//                continue;
//
//            // Building the full path where the image will be saved ofr 24bits-JPEG
//            BeTest::GetHost().GetOutputRoot(outputFilePath);
//            outputFilePath.AppendToPath(L"24Bits_IJG_Jpeg");
//            BeFileName SourceFile(GetParam().c_str());
//            outputFilePath.AppendToPath(SourceFile.GetExtension().c_str());
//
//            //Create the ouput folder if not already created
//            if (!outputFilePath.DoesPathExist())
//                ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(outputFilePath.GetName()));
//
//            outputFilePath.AppendToPath(SourceFile.GetFileNameWithoutExtension().c_str());
//            outputFilePath.append(L"_");
//            outputFilePath.append(s_GetBlockNumberToChar(pCurrentBlock->GetBlockType()));
//
//            HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outputFilePath.GetNameUtf8());
//
//            exporter->SelectExportFilename(pURL);
//            exporter->SelectPixelType(pPixel24Bits->GetPixelTypeClassID());
//            exporter->SelectCodec(pCodecJpeg->GetCodecClassID());
//            exporter->SelectBlockType(pCurrentBlock->GetBlockType());
//
//
//            // Exporting
//            HFCPtr<HRFRasterFile> pFile = exporter->StartExport();
//            if (HasFailure())
//                return;
//            ASSERT_TRUE(pFile.GetPtr() != nullptr);
//            //ASSERT_NO_THROW(exporter->StartExport());            
//            }
//        }
//    catch (...)
//        {
//        FAIL();
//        }
//    }
//
//
////:>--------------------------------------------------------------------------------------+
//// Test the export from a raster file to iTiff file with :
////      All the possible block types
////      32 bits Pixel Type assicated with Deflate Codec (see HCDCodecZlib)
////                                                Laurent.Robert-Veillette            04/2016
////:>+--------------------------------------------------------------------------------------
//TEST_P(ExportTester, ExportToiTiff32BitsDeflate)
//    {
//    //Test that the source paths is not empty
//    ASSERT_FALSE(GetParam().empty());
//
//    HRFiTiffCreator* creator = HRFiTiffCreator::GetInstance();
//    ASSERT_TRUE(creator->GetSupportedAccessMode().m_HasCreateAccess);
//
//    HFCPtr<HFCURL> UrlSource = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GetParam().c_str()));
//
//    try
//        {
//        //N.B. We set the 4th parameter ScanCreatorIfNotFound = true because some files do not have the good extension. 
//        //Hence we cannot rely only on the extension to find the appropriate creator.
//        HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE, 0, true, false);
//        if (pSrcCreator == nullptr)
//            return;
//
//        HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
//        //Verify that the Rasterfile created is valid
//        ASSERT_FALSE(nullptr == pRasterFileSource);
//
//        HFCPtr<HGF2DWorldCluster> worldCluster = new HGFHMRStdWorldCluster();
//
//        //Get all pixel types available for this creator
//        const HFCPtr<HRFRasterFileCapabilities> PixelCapabilities = creator->GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID, HFC_CREATE_ONLY);
//        ASSERT_FALSE(nullptr == PixelCapabilities);
//
//        //Create the pixel type 32 bits
//        HFCPtr<HRFPixelTypeCapability> pPixel32Bits = new HRFPixelTypeCapability(HFC_CREATE_ONLY, HRPPixelTypeV32R8G8B8A8::CLASS_ID, new HRFRasterFileCapabilities());
//        pPixel32Bits = static_cast<HRFPixelTypeCapability*>(PixelCapabilities->GetCapabilityOfType(((HFCPtr<HRFCapability>)pPixel32Bits)).GetPtr());
//        ASSERT_FALSE(nullptr == pPixel32Bits);
//
//        //Create the codec Deflate (ZLib)
//        const HFCPtr<HRFRasterFileCapabilities> CodecCapabilities = pPixel32Bits->GetCodecCapabilities();
//        ASSERT_FALSE(nullptr == CodecCapabilities);
//
//        HFCPtr<HRFCodecCapability> pCodecDeflate = new HRFCodecCapability(HFC_CREATE_ONLY, HCDCodecZlib::CLASS_ID, new HRFRasterFileCapabilities());
//        pCodecDeflate = static_cast<HRFCodecCapability*>(CodecCapabilities->GetCapabilityOfType(((HFCPtr<HRFCapability>)pCodecDeflate)).GetPtr());
//        ASSERT_FALSE(nullptr == pCodecDeflate);
//
//        //Iterate on all block types for 32 bits - Deflate (ZLib)
//        BeFileName outputFilePath;
//        std::unique_ptr<HUTImportFromFileExportToFile> exporter(new HUTImportFromFileExportToFile(worldCluster));
//
//        exporter->SetImportRasterFile(pRasterFileSource);
//        exporter->SelectExportFileFormat(creator);
//
//        const HFCPtr<HRFRasterFileCapabilities> BlockCapabilities = pCodecDeflate->GetBlockTypeCapabilities();
//        ASSERT_FALSE(nullptr == BlockCapabilities);
//
//        for (uint32_t blockIndex(0); blockIndex < BlockCapabilities->CountCapabilities(); ++blockIndex)
//            {
//            const HFCPtr<HRFBlockCapability> pCurrentBlock = (const HFCPtr<HRFBlockCapability>&) BlockCapabilities->GetCapability(blockIndex);
//            ASSERT_FALSE(nullptr == pCurrentBlock);
//
//            // If it is not possible to write a new file using the current block type, we skip it
//            if (!pCurrentBlock->GetAccessMode().m_HasCreateAccess)
//                continue;
//
//            // Building the full path where the image will be saved ofr 32 bits - Deflate (ZLib)
//            BeTest::GetHost().GetOutputRoot(outputFilePath);
//            outputFilePath.AppendToPath(L"32Bits_Deflate");
//            BeFileName SourceFile(GetParam().c_str());
//            outputFilePath.AppendToPath(SourceFile.GetExtension().c_str());
//
//            //Create the ouput folder if not already created
//            if (!outputFilePath.DoesPathExist())
//                ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(outputFilePath.GetName()));
//
//            outputFilePath.AppendToPath(SourceFile.GetFileNameWithoutExtension().c_str());
//            outputFilePath.append(L"_");
//            outputFilePath.append(s_GetBlockNumberToChar(pCurrentBlock->GetBlockType()));
//
//            HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outputFilePath.GetNameUtf8());
//
//            exporter->SelectExportFilename(pURL);
//            exporter->SelectPixelType(pPixel32Bits->GetPixelTypeClassID());
//            exporter->SelectCodec(pCodecDeflate->GetCodecClassID());
//            exporter->SelectBlockType(pCurrentBlock->GetBlockType());
//
//
//            // Exporting
//            HFCPtr<HRFRasterFile> pFile = exporter->StartExport();
//            if (HasFailure())
//                return;
//            ASSERT_TRUE(pFile.GetPtr() != nullptr);
//            //ASSERT_NO_THROW(exporter->StartExport());
//            }
//        }
//    catch (...)
//        {
//        FAIL();
//        }
//    }
//
//
//
////:>--------------------------------------------------------------------------------------+
////
//// Test the export from a raster file to iTiff file. Iterates on every possible options of 
//// pixel type, codec and block type.
////                                             Laurent.Robert-Veillette            04/2016
////:>+--------------------------------------------------------------------------------------
//TEST_P(ExportTester, FromFileToiTiffExportAllOptions)
//    {
//    ASSERT_FALSE(GetParam().empty());   // Test that the source paths is not empty
//
//    HRFiTiffCreator* creator = HRFiTiffCreator::GetInstance();
//    ASSERT_TRUE(creator->GetSupportedAccessMode().m_HasCreateAccess);
//
//    HFCPtr<HFCURL> UrlSource = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GetParam().c_str()));
//
//    try
//        {
//        //N.B. We set the 4th parameter ScanCreatorIfNotFound = true because some files do not have the good extension. 
//        //Hence we cannot rely only on the extension to find the appropriate creator.
//        HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE, 0, true, false);
//        //&&MM find a way to ignore some file extension? ex: world file?? if it throws that generates a failure
//        // even if we were able to ignore exceptions we might be ignoring a real exception
//        if (pSrcCreator == nullptr)
//            return;
//
//        //&&MM reopen the source for each iteration? Need to determine what we really want to test.
//        //      if we want to test read of source why bother exporting to all itiff for every file aren't we loosing our time?
//        HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
//
//        //Verify that the Rasterfile created is valid
//        ASSERT_FALSE(nullptr == pRasterFileSource);
//
//        // Getting pixel type capabilities
//        const HFCPtr<HRFRasterFileCapabilities> capabilities = creator->GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID, HFC_CREATE_ONLY);
//
//        HFCPtr<HGF2DWorldCluster> worldCluster = new HGFHMRStdWorldCluster();
//
//        BeFileName outputFilePath;
//        BeTest::GetHost().GetOutputRoot(outputFilePath);
//        BeFileName::CreateNewDirectory(outputFilePath.c_str());
//
//        //Run the export on every possible pixel types, codecs and block types
//        for (uint32_t ptIndex(0); ptIndex < capabilities->CountCapabilities(); ++ptIndex)
//            {
//            const HFCPtr<HRFPixelTypeCapability> pCurrentPixelType = (const HFCPtr<HRFPixelTypeCapability>&)(capabilities->GetCapability(ptIndex));
//
//            // If it is not possible to write a new file using the current pixel type, we skip it
//            if (!pCurrentPixelType->GetAccessMode().m_HasCreateAccess)
//                continue;
//
//            // Getting codec capabilities for every pixel type
//            const HFCPtr<HRFRasterFileCapabilities> codecCaps = pCurrentPixelType->GetCodecCapabilities();
//            for (uint32_t codecIndex(0); codecIndex < codecCaps->CountCapabilities(); ++codecIndex)
//                {
//                const HFCPtr<HRFCodecCapability> pCurrentCodec = (const HFCPtr<HRFCodecCapability>&) codecCaps->GetCapability(codecIndex);
//
//                // If it is not possible to write a new file using the current codec, we skip it too
//                if (!pCurrentCodec->GetAccessMode().m_HasCreateAccess)
//                    continue;
//
//                const HFCPtr<HRFRasterFileCapabilities> blockCaps = pCurrentCodec->GetBlockTypeCapabilities();
//                for (uint32_t blockIndex(0); blockIndex < blockCaps->CountCapabilities(); ++blockIndex)
//                    {
//                    const HFCPtr<HRFBlockCapability> pCurrentBlock = (const HFCPtr<HRFBlockCapability>&) blockCaps->GetCapability(blockIndex);
//
//                    // If it is not possible to write a new file using the current block type, we skip it too
//                    if (!pCurrentBlock->GetAccessMode().m_HasCreateAccess)
//                        continue;
//
//                    // Building the full path where the image will be saved                
//                    BeFileName ID = s_GetProfileID(pRasterFileSource,
//                                                   pCurrentPixelType->GetPixelTypeClassID(),
//                                                   pCurrentCodec->GetCodecClassID(),
//                                                   pCurrentBlock->GetBlockType());
//
//
//
//                    BeFileName SourceFile(GetParam().c_str());
//                    BeFileName outfilename(outputFilePath);
//                    outfilename.AppendToPath(SourceFile.GetFileNameWithoutExtension().c_str());
//                    outfilename.append(L"_");
//                    outfilename.append(ID);
//                    //newName.AppendUtf8(creator->GetDefaultExtension().c_str());
//                    //outfilename.append(newName);
//
//                    //outputFilePath.AppendToPath(filename.c_str());
//                    HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outfilename.GetNameUtf8());
//
//                    std::unique_ptr<HUTImportFromFileExportToFile> exporter(new HUTImportFromFileExportToFile(worldCluster));
//
//                    exporter->SetImportRasterFile(pRasterFileSource);
//                    exporter->SelectExportFileFormat(creator);
//                    exporter->SelectExportFilename(pURL);
//                    exporter->SelectPixelType(pCurrentPixelType->GetPixelTypeClassID());
//                    exporter->SelectCodec(pCurrentCodec->GetCodecClassID());
//                    exporter->SelectBlockType(pCurrentBlock->GetBlockType());
//
//
//                    // Exporting
//                    HFCPtr<HRFRasterFile> pFile = exporter->StartExport();
//                    ASSERT_TRUE(pFile.GetPtr() != nullptr);
//                    //ASSERT_NO_THROW(exporter->StartExport());
//                    }
//                }
//            }
//        }
//    catch (...)
//        {
//        FAIL();
//        }
//    }

//:>--------------------------------------------------------------------------------------+
//
//Instantiate (run) all the TEST_P of ExportTester with the files found in the argument path
//
//:>+--------------------------------------------------------------------------------------
INSTANTIATE_TEST_CASE_P(AllRastersInDirectory, ExportTester,
                        ::testing::ValuesIn(s_GetFileNameVector()));    

// #else
// 
// #pragma message("Warning: Disabling ExportTester because TEST_P/INSTANTIATE_TEST_CASE_P are not available")

//#endif
