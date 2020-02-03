//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : ExportTester
//-----------------------------------------------------------------------------

#include <ImagePP/all/h/HPMPool.h>
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>
#include <ImagePP/all/h/HFCURL.h>
#include <Bentley/md5.h>
#include <json/json.h>
#include <ImagePP/all/h/HRFRasterFile.h>

// Clear timestamp related info for certain format so we can compare them.
bool UpdateTiffHistogramTimestamp(ImagePP::HFCPtr<ImagePP::HFCURL> const& pUrl);


/*=================================================================================**//**
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct RasterTestInfo
    {
    public:
        RasterTestInfo(BeFileNameCR rasterPath);
        ~RasterTestInfo(){}

        bool Load();
        bool Store();

        bool         ComputeMD5();
        Utf8StringCR GetMD5() const { return m_md5; }
        void         SetMD5(Utf8CP val) { m_md5 = val; }

        uint64_t     GetExportDuration() const { return m_exportDuration; }
        void         SetExportDuration(uint64_t val) { m_exportDuration = val; }
    
        BeFileNameCR GetInfoFilename() const {return m_infoPath;}

        Utf8StringCR GetBuildType() const { return m_buildType; }

        Utf8StringCR GetComputerName() const { return m_computerName; }

    private:
        Utf8String GetImagePPSourcePath() const;

        BeFileName m_rasterPath;
        BeFileName m_infoPath;
        Utf8String m_md5;
        uint64_t   m_exportDuration;
        Utf8String m_buildType;
        Utf8String m_computerName;
    };


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
template<class TestingBase_T>
struct ExporterTestFixture : TestingBase_T
    {
    public:
        ExporterTestFixture()
            {
            m_pWorldCluster = new HGFHMRStdWorldCluster();
            }

        ~ExporterTestFixture()
            {};

        virtual void SetUp() override { ImagePPTestConfig::GetConfig().SetUp(); }

        ImagePP::HPMPool& GetPool()
            {
            static HPMPool* s_pPool = new ImagePP::HPMPool(200 * 1024, nullptr);
            return *s_pPool;
            }

        ImagePP::HFCPtr<ImagePP::HGF2DWorldCluster> GetWorld() { return m_pWorldCluster; }

        //----------------------------------------------------------------------------------------
        // @bsimethod                                                   Mathieu.Marchand  6/2016
        //----------------------------------------------------------------------------------------
        BeFileName BuildRelativeOutputFileName(WCharCP testName, BeFileNameCR sourceFile, ImagePP::HRFRasterFileCreator const& creator)
            {
            BeFileName sourceDir = ImagePPTestConfig::GetConfig().GetSourceDir();

            // Extract sub-dir part from sourceFile
            BeFileName relativePath;
            BeFileName::FindRelativePath(relativePath, sourceFile.c_str(), sourceDir.c_str());
            if (relativePath == sourceFile)
                return BeFileName();

            BeAssert(relativePath != sourceFile);

            WString defaultExtension(creator.GetDefaultExtension().c_str(), BentleyCharEncoding::Utf8);
            if (defaultExtension.StartsWith(L"*."))
                defaultExtension.erase(0, 2);

            BeFileName relativeFilename(testName);
            relativeFilename.AppendToPath(relativePath);
            relativeFilename.AppendExtension(defaultExtension.c_str());

            return relativeFilename;
            }

    private:
        ImagePP::HFCPtr<ImagePP::HGF2DWorldCluster> m_pWorldCluster;

    };
