//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/FileFormat/FileFormatTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : ExportTester
//-----------------------------------------------------------------------------

#include "../imagepptestpch.h"

//#ifdef USE_GTEST        // TEST_P only available when using gtest.

#include <Bentley/BeDirectoryIterator.h>


#include <Imagepp/all/h/HUTClassIDDescriptor.h>

//Files
#include <Bentley/BeFileName.h>
#include <ImagePP/all/h/HRFFileFormats.h>

//SQLite and SQLang
#include <BeSQLite/L10N.h>
#include <BeSQLite/BeSQLite.h>


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
class ExportTester : public ::testing::TestWithParam< Utf8String >
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

/*---------------------------------------------------------------------------------**//**
* Return a vector with all the paths to the rasters.
* @bsimethod                                             Laurent.Robert-Veillette 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static vector<Utf8String> s_GetDirectoryVector(const Utf8String& pSourceDir)
    {
    vector<Utf8String> directoryList;

    BeFileName sourcePath(pSourceDir);
    BeFileName actualPath;
    bool isDir;
    for (BeDirectoryIterator dir(sourcePath); dir.GetCurrentEntry(actualPath, isDir, true) == SUCCESS; dir.ToNext())
        {
        if (isDir)
            continue;//What do we do?? Skip?
        else
            directoryList.push_back(actualPath.GetNameUtf8());
        }

    return directoryList;
    }

/*---------------------------------------------------------------------------------**//**
* Format the name of the file with the specific options of the export
* @bsimethod                                         Laurent.Robert-Veillette     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName GetProfileID(const HFCPtr<HRFRasterFile>&   pi_rpRasterFile,
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

//:>--------------------------------------------------------------------------------------+
//
// Test the export from a raster file to iTiff file. Iterates on every possible options of 
// pixel type, codec and block type.
//
//:>+--------------------------------------------------------------------------------------
TEST_P(ExportTester, FromFileToiTiffExportAllOptions)
    {
    ASSERT_FALSE(GetParam().empty());   // Test that the source paths is not empty

    HRFiTiffCreator* creator = HRFiTiffCreator::GetInstance();
    ASSERT_TRUE(creator->GetSupportedAccessMode().m_HasCreateAccess);

    HFCPtr<HFCURL> UrlSource = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + GetParam().c_str());
    HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    //&&MM find a way to ignore some file extension? ex: world file?? if it throws that generates a failure
    // even if we were able to ignore exceptions we might be ignoring a real exception
    if (pSrcCreator == nullptr)
        return;

    //&&MM reopen the source for each iteration? Need to determine what we really want to test.
    //      if we want to test read of source why bother exporting to all itiff for every file aren't we loosing our time?
    HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(UrlSource, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    
    //Verify that the Rasterfile created is valid
    ASSERT_FALSE(nullptr == pRasterFileSource);    

    // Getting pixel type capabilities
    const HFCPtr<HRFRasterFileCapabilities> capabilities = creator->GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID, HFC_CREATE_ONLY);

    HFCPtr<HGF2DWorldCluster> worldCluster = new HGFHMRStdWorldCluster();

    BeFileName outputFilePath;
    BeTest::GetHost().GetOutputRoot(outputFilePath);
    BeFileName::CreateNewDirectory(outputFilePath.c_str());

    //Run the export on every possible pixel types, codecs and block types
    for (uint32_t ptIndex(0); ptIndex < capabilities->CountCapabilities(); ++ptIndex)
        {
        const HFCPtr<HRFPixelTypeCapability> pCurrentPixelType = (const HFCPtr<HRFPixelTypeCapability>&)(capabilities->GetCapability(ptIndex));

        // If it is not possible to write a new file using the current pixel type, we skip it
        if (!pCurrentPixelType->GetAccessMode().m_HasCreateAccess)
            continue;

        // Getting codec capabilities for every pixel type
        const HFCPtr<HRFRasterFileCapabilities> codecCaps = pCurrentPixelType->GetCodecCapabilities();
        for (uint32_t codecIndex(0); codecIndex < codecCaps->CountCapabilities(); ++codecIndex)
            {
            const HFCPtr<HRFCodecCapability> pCurrentCodec = (const HFCPtr<HRFCodecCapability>&) codecCaps->GetCapability(codecIndex);

            // If it is not possible to write a new file using the current codec, we skip it too
            if (!pCurrentCodec->GetAccessMode().m_HasCreateAccess)
                continue;

            const HFCPtr<HRFRasterFileCapabilities> blockCaps = pCurrentCodec->GetBlockTypeCapabilities();
            for (uint32_t blockIndex(0); blockIndex < blockCaps->CountCapabilities(); ++blockIndex)
                {
                const HFCPtr<HRFBlockCapability> pCurrentBlock = (const HFCPtr<HRFBlockCapability>&) blockCaps->GetCapability(blockIndex);

                // If it is not possible to write a new file using the current block type, we skip it too
                if (!pCurrentBlock->GetAccessMode().m_HasCreateAccess)
                    continue;

                // Building the full path where the image will be saved                
                BeFileName ID = GetProfileID(pRasterFileSource,
                                             pCurrentPixelType->GetPixelTypeClassID(),
                                             pCurrentCodec->GetCodecClassID(),
                                             pCurrentBlock->GetBlockType());
                
                

                BeFileName SourceFile(GetParam());
                BeFileName outfilename(outputFilePath);
                outfilename.AppendToPath(SourceFile.GetFileNameWithoutExtension().c_str());
                outfilename.append(L"_");
                outfilename.append(ID);
                //newName.AppendUtf8(creator->GetDefaultExtension().c_str());
                //outfilename.append(newName);

                //outputFilePath.AppendToPath(filename.c_str());
                HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outfilename.GetNameUtf8());
                //????????????????????????????????????????????????????????????????????????????????????????????????????????????
                                  
                HUTImportFromFileExportToFile exporter(worldCluster);

                exporter.SetImportRasterFile(pRasterFileSource);
                exporter.SelectExportFileFormat(creator);
                exporter.SelectExportFilename(pURL);
                exporter.SelectPixelType(pCurrentPixelType->GetPixelTypeClassID());
                exporter.SelectCodec(pCurrentCodec->GetCodecClassID());
                exporter.SelectBlockType(pCurrentBlock->GetBlockType());

                try
                    {
                    // Exporting
                    HFCPtr<HRFRasterFile> pFile = exporter.StartExport();
                    ASSERT_TRUE(pFile.GetPtr() != nullptr);
                    //ASSERT_NO_THROW(exporter.StartExport());
                    }                
                catch (...)
                    {
                    //&&MM filter what are common error and what are not.
                    }                
                }
            }
        }
    }

//:>--------------------------------------------------------------------------------------+
//
//Instantiate (run) all the TEST_P of ExportTester with the files found in the argument path
//
//:>+--------------------------------------------------------------------------------------
INSTANTIATE_TEST_CASE_P(AllRastersInDirectory, ExportTester, 
                        ::testing::ValuesIn(s_GetDirectoryVector("C:\\_NOT_NOW_images\\BMP\\")));

////:>--------------------------------------------------------------------------------------+
//// Constructor of the fixture class
////:>+--------------------------------------------------------------------------------------
//ExportTester::ExportTester()
//    {
//    //Iterate on all the files and also looking in subdirectories
//    WString filePathList(pDirSource.c_str(), BentleyCharEncoding::Utf8);
//    BeFileListIterator FileList(filePathList, true);
//    BeFileName fileName;
//    while (FileList.GetNextFileName(fileName) == SUCCESS)
//        {
//        //Trying to construct a HRFRasterFile  
//        HFCPtr<HFCURL> FileUrlPtr = HFCURL::CreateFrom(fileName);
//        HFCPtr<HRFRasterFile> ActualRasterFilePtr = new HRFRasterFile(FileUrlPtr);
//
//        //Verify if the raster has been created and push it into m_RasterfileVector
//        if(ActualRasterFilePtr)
//            m_RasterfileVector.push_back(ActualRasterFilePtr);
//        }
//    }

// #else
// 
// #pragma message("Warning: Disabling ExportTester because TEST_P/INSTANTIATE_TEST_CASE_P are not available")

//#endif
