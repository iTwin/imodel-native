//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/FileFormat/ToBestiTiffTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : ExportiTiffTester
//-----------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include <ImagePP/h/ImageppAPI.h>
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>
#include <ImagePP/all/h/HUTImportFromFileExportToFile.h>
#include <ImagePP/all/h/HUTImportFromRasterExportToFile.h>
#include "../Common/ImagePPTestConfig.h"
#include "ExporterTestFixture.h"
#include <Bentley/BeFileName.h>
#include <Imagepp/all/h/HPMPool.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HRFiTiffFile.h>


USING_NAMESPACE_IMAGEPP
//#ifdef USE_GTEST        // TEST_P only available when using gtest.

/*---------------------------------------------------------------------------------**//**
* Class definition
* @bsimethod                                             Laurent.Robert-Veillette 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
class ExportiTiffTester : public ExporterTestFixture<::testing::TestWithParam< std::wstring >> /*We use std::wstring to get a proper display from gtest*/
    {
    public:
        ExportiTiffTester() {}
        virtual ~ExportiTiffTester() {};

    private:
    };

//:>--------------------------------------------------------------------------------------+
//
// Test the export from a raster file to iTiff file with the best possible options.
//                                                Laurent.Robert-Veillette            04/2016
//:>+--------------------------------------------------------------------------------------
TEST_P(ExportiTiffTester, ToBestiTiff)
    {
    //Test that the source paths is not empty
    ASSERT_FALSE(GetParam().empty());
    ASSERT_TRUE(GeoCoordinates::BaseGCS::IsLibraryInitialized());

    BeFileName sourceFileName(GetParam().c_str());

    HFCPtr<HFCURL> pSourceUrl = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(sourceFileName.GetNameUtf8().c_str()));


    try
        {
        //N.B. We set the 4th parameter ScanCreatorIfNotFound = true because some files do not have the good extension. 
        //Hence we cannot rely only on the extension to find the appropriate creator.
        //>>> turning it OFF for now. It is slow. We fixed file extention. -MM
        bool scanCreator = false;
        HRFRasterFileCreator const* pSrcCreator = HRFRasterFileFactory::GetInstance()->FindCreator(pSourceUrl, HFC_READ_ONLY | HFC_SHARE_READ_WRITE, 0, scanCreator, false);
        if (pSrcCreator == nullptr)
            return;

        printf("[ TRACE    ] Exporting: %s\n", sourceFileName.GetNameUtf8().c_str());

        HFCPtr<HRFRasterFile> pRasterFileSource = pSrcCreator->Create(pSourceUrl, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
        //Verify that the Rasterfile created is valid
        ASSERT_FALSE(nullptr == pRasterFileSource);

#if 1   // Old base line was created with file to file and that will copy extra meta data information. we need them to compare with old baseline.
        std::unique_ptr<HUTImportFromFileExportToFile> exporter(new HUTImportFromFileExportToFile(GetWorld()));
        exporter->SelectExportFileFormat(HRFiTiffCreator::GetInstance());
        exporter->SetImportRasterFile(pRasterFileSource);

#else        
        HFCPtr<HGF2DCoordSys> pLogical = GetWorld()->GetCoordSysReference(pRasterFileSource->GetPageWorldIdentificator(0));
        HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(&GetPool(), pRasterFileSource, 0, pLogical);
        ASSERT_TRUE(pStore != nullptr);
        HFCPtr<HRARaster> pRaster = pStore->LoadRaster();
        ASSERT_TRUE(pRaster != nullptr);

        std::unique_ptr<HUTImportFromRasterExportToFile> exporter(new HUTImportFromRasterExportToFile(pRaster, *pRaster->GetEffectiveShape(), GetWorld()));
        exporter->SelectExportFileFormat(HRFiTiffCreator::GetInstance());

#endif
        auto& config = ImagePPTestConfig::GetConfig();

        // Use BestMatchSelectedValues for exportation
        ASSERT_NO_THROW(exporter->BestMatchSelectedValues());

        //&&MM best match should use fax4 for binary and not RLE. make sure it is strip also.
        //      - select NEAREAST instead of average?  might run faster
        //      - MrSid is very slow. reduce dataset.

        BeFileName relativeOutFilename = BuildRelativeOutputFileName(L"ExportToBestiTiff", sourceFileName, *exporter->GetSelectedExportFileFormat());
        
        BeFileName outputFilePath(config.GetFileFormatOutputDir());
        ASSERT_FALSE(outputFilePath.IsEmpty());

        // Build output: outputRoot + testName + sourceSubDir + outputExtension
        // ex: ....\Product\ImagePP-GTest\run\Output\ + ExportiTiffTester\toBestiTiff\ + Image\jpeg\24bit.jpg + .iTiff
        outputFilePath.AppendToPath(relativeOutFilename);

        if (!outputFilePath.GetDirectoryName().DoesPathExist())
            ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(outputFilePath.GetDirectoryName().c_str()));

        HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + outputFilePath.GetNameUtf8());
        exporter->SelectExportFilename(pURL);

        uint64_t startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        HFCPtr<HRFRasterFile> pFile = exporter->StartExport();
        exporter.reset();
        ASSERT_TRUE(pFile.GetPtr() != nullptr);
        pFile = nullptr;    // Close the RasterFile in order to be able to open it for UpdateTiffHistogramTimestamp()
        uint64_t endTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

        //*** Generate export info...
        RasterTestInfo outputInfo(outputFilePath);

        static bool s_writeTestInfo = true;
        if (s_writeTestInfo)
            {
            outputInfo.SetExportDuration(endTime - startTime);

            ASSERT_TRUE(outputInfo.ComputeMD5());
            ASSERT_TRUE(outputInfo.Store());
            }
        
        // *** validate against baseline.
        if (config.CompareAgainsBaseline())
            {
            BeFileNameCR baseLineDir = config.GetBaselineDir();
            ASSERT_TRUE(baseLineDir.DoesPathExist());

            BeFileName baselineFilePath = baseLineDir;
            baselineFilePath.AppendToPath(relativeOutFilename);

            if (!baselineFilePath.DoesPathExist())
                FAIL() << "Baseline specified but file is missing : " << baselineFilePath.GetNameUtf8().c_str();
                
            RasterTestInfo baselineInfo(baselineFilePath);

            if (!baselineInfo.Load())
                FAIL() << "Baseline info file is missing : " << baselineInfo.GetInfoFilename().c_str();
                
            if (config.DoMd5())
                ASSERT_STREQ(baselineInfo.GetMD5().c_str(), outputInfo.GetMD5().c_str()) << "MD5 check failed";

            if (config.ValidateExportDuration())
                {
                EXPECT_STREQ(baselineInfo.GetBuildType().c_str(), outputInfo.GetBuildType().c_str()) << "Build must be of same build type for duration validation";
                EXPECT_STREQ(baselineInfo.GetComputerName().c_str(), outputInfo.GetComputerName().c_str()) << "Baseline must be from the same machine for duration validation";

                if (outputInfo.GetExportDuration() > config.GetDuratationThreshold())
                    {
                    double exportDelta = (double) outputInfo.GetExportDuration() - (double) baselineInfo.GetExportDuration();
                    double exportRatio = fabs(exportDelta) / baselineInfo.GetExportDuration();
                    ASSERT_LE(exportRatio, config.GetToleranceRatio())
                        << "Base time: " << baselineInfo.GetExportDuration() << "ms New time: " << outputInfo.GetExportDuration() << "ms";
                    }
                }
            }        
        }
    catch (HFCException& e)
        {
        FAIL() << e.GetExceptionMessage().c_str();
        }
    }

INSTANTIATE_TEST_CASE_P(FormatTests, ExportiTiffTester, ::testing::ValuesIn(ImagePPTestConfig::GetConfig().GetSourceList()));

// #else
// 
// #pragma message("Warning: Disabling ExportiTiffTester because TEST_P/INSTANTIATE_TEST_CASE_P are not available")

//#endif
