//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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
#include <ImagePP/all/h/HPMPool.h>
#include <ImagePP/all/h/HRSObjectStore.h>
#include <ImagePP/all/h/HRFiTiffFile.h>
#include <Bentley/BeDirectoryIterator.h>
#include <ImagePP/all/h/HPSObjectStore.h>


USING_NAMESPACE_IMAGEPP
//#ifdef USE_GTEST        // TEST_P only available when using gtest.

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
class PssTester : public ExporterTestFixture<::testing::TestWithParam< std::wstring >> /*We use std::wstring to get a proper display from gtest*/
    {
    public:
        PssTester() {}
        virtual ~PssTester() {};

        //----------------------------------------------------------------------------------------
        // @bsimethod                                                   Mathieu.Marchand  10/2016
        //----------------------------------------------------------------------------------------
        static std::list<std::wstring> BuildFileList()
            {
            BeFileName sourceDir = ImagePPTestConfig::GetConfig().GetSourceDir();
            sourceDir.AppendToPath(L"PSS");

            std::list<std::wstring> pssFileList;
            bvector<BeFileName> fileList;

            const WString glob = L"*";

            BeDirectoryIterator::WalkDirsAndMatch(fileList, sourceDir, glob.c_str(), true);
            for (auto& actualName : fileList)
                {
                if (actualName.GetExtension().EqualsI(L"pss"))
                    pssFileList.push_back(actualName.GetName());
                }

            return pssFileList;
            }

    private:
    };


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
TEST_P(PssTester, Export)
    {
    //Test that the source paths is not empty
    ASSERT_FALSE(GetParam().empty());
    ASSERT_TRUE(GeoCoordinates::BaseGCS::IsLibraryInitialized());

    BeFileName sourceFileName(GetParam().c_str());

    HFCPtr<HFCURL> pSourceUrl = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(sourceFileName.GetNameUtf8().c_str()));


    try
        {
        auto& config = ImagePPTestConfig::GetConfig();

        printf("[ TRACE    ]   Testing: %s\n", sourceFileName.GetNameUtf8().c_str());

        HFCPtr<HPSObjectStore> pStore = new HPSObjectStore(&GetPool(), pSourceUrl, GetWorld());
        ASSERT_TRUE(pStore != nullptr);

        HFCPtr<HRARaster> pRaster = pStore->LoadRaster(0);
        ASSERT_TRUE(pRaster != nullptr);

        std::unique_ptr<HUTImportFromRasterExportToFile> exporter(new HUTImportFromRasterExportToFile(pRaster, *pRaster->GetEffectiveShape(), GetWorld()));
        exporter->SelectExportFileFormat(HRFiTiffCreator::GetInstance());
      
        // Use BestMatchSelectedValues for exportation
        ASSERT_NO_THROW(exporter->BestMatchSelectedValues());

        exporter->SetMaintainAspectRatio(true);
        exporter->SetImageWidth(256);

        BeFileName relativeOutFilename = BuildRelativeOutputFileName(L"ExportPss", sourceFileName, *exporter->GetSelectedExportFileFormat());

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
        outputInfo.SetExportDuration(endTime - startTime);

        ASSERT_TRUE(outputInfo.ComputeMD5());
        ASSERT_TRUE(outputInfo.Store());
        
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
                    double exportRatio = exportDelta / baselineInfo.GetExportDuration();
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

INSTANTIATE_TEST_CASE_P(FormatTests, PssTester, ::testing::ValuesIn(PssTester::BuildFileList()));

// #else
// 
// #pragma message("Warning: Disabling ExportiTiffTester because TEST_P/INSTANTIATE_TEST_CASE_P are not available")

//#endif
