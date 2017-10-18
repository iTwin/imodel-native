/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Converters/TiledConverterApp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbSync/DgnV8/ConverterApp.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/15
//=======================================================================================
struct TiledConverterApp : ConverterApp
{
    DEFINE_T_SUPER(ConverterApp)
private:
    SpatialConverterBase::SpatialParams m_params;
    bvector<BeFileName> m_inputSpec;
    std::unique_ptr<TiledFileConverter> m_converter;
    
    Converter::Params& _GetConverterParams() override {return m_params;}

    // iModelBridge:
    void _PrintUsage() override;
    iModelBridge::Params& _GetParams() override {return m_params;}
    CmdLineArgStatus _ParseCommandLineArg(int iArg, int argc, WCharCP argv[]);
    BentleyStatus _Initialize(int argc, WCharCP argv[]) override;
    BentleyStatus _ConvertToBim(Dgn::SubjectCR jobSubject) override;
    Dgn::SubjectCPtr _InitializeJob() override;
    BentleyStatus _OnConvertToBim(DgnDbR db) override;
    void _OnConvertedToBim(BentleyStatus) override;
    Dgn::SubjectCPtr _FindJob() override;
    BentleyStatus _OpenSource() override;
    void _CloseSource(BentleyStatus) override;
    BentleyStatus _OnRootFilesConverted() override;

public:
    TiledConverterApp() {}
    ~TiledConverterApp() {BeAssert(nullptr == m_converter.get());}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledConverterApp::_PrintUsage()
    {
    fwprintf(stderr,
L"  --tiles=                (required)  A directory or wildcard specfication for the tile files. May appear multiple times.\n\
%s\n", GetCommonCommandLineOptions().c_str()
    );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge::CmdLineArgStatus TiledConverterApp::_ParseCommandLineArg(int iArg, int argc, WCharCP argv[]) 
    {
    if (argv[iArg] == wcsstr(argv[iArg], L"--tiles="))
        {
        m_inputSpec.push_back(BeFileName(GetArgValueW(argv[iArg])));
        return CmdLineArgStatus::Success;
        }

    return T_Super::_ParseCommandLineArg(iArg, argc, argv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledConverterApp::_Initialize(int argc, WCharCP argv[])
    {
    m_params.SetInputFileName(_GetParams().GetInputFileName());
    return T_Super::_Initialize(argc, argv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledConverterApp::_OpenSource()
    {
    auto initStat = m_converter->InitRootModel();
    if (DgnV8Api::DGNFILE_STATUS_Success != initStat)
        {
        m_converter->ReportDgnV8FileOpenError(initStat, _GetParams().GetInputFileName().c_str());
        return BentleyStatus::ERROR;
        }
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledConverterApp::_CloseSource(BentleyStatus)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledConverterApp::_OnConvertToBim(DgnDbR db)
    {
    m_converter.reset(new TiledFileConverter(m_params));
    m_converter->SetDgnDb(db);
    return m_converter->AttachSyncInfo();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledConverterApp::_OnConvertedToBim(BentleyStatus)
    {
    m_converter.reset(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr TiledConverterApp::_FindJob()
    {
    auto status = m_converter->FindJob();
    return (SpatialConverterBase::ImportJobLoadStatus::Success == status)? &m_converter->GetImportJob().GetSubject(): nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr TiledConverterApp::_InitializeJob()
    {
    auto status = m_converter->InitializeJob();

    return (SpatialConverterBase::ImportJobCreateStatus::Success == status)? &m_converter->GetImportJob().GetSubject(): nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledConverterApp::_OnRootFilesConverted()
    {
    m_converter->DetectDeletedFiles();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledConverterApp::_ConvertToBim(Dgn::SubjectCR jobSubject)
    {
    BeAssert(&jobSubject == &m_converter->GetImportJob().GetSubject());

    m_converter->ConvertRootModel();
    if (m_converter->WasAborted())
        return BentleyStatus::ERROR;

    for (BeFileName const& input : m_inputSpec)
        {
        BeFileListIterator inputFiles (input, true);
        BeFileName inputFilePath;
        while (BSISUCCESS == inputFiles.GetNextFileName(inputFilePath))
            {
            if (!BeFileName::IsDirectory(inputFilePath))
                {
                m_converter->ConvertTile(inputFilePath);
                if (m_converter->WasAborted())
                    return BentleyStatus::ERROR;
                }
            }
        }

    if (m_converter->WasAborted())
        return BSIERROR;

    m_converter->FinishedConversion();

    return BentleyStatus::SUCCESS;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, wchar_t const* argv[])
    {
    TiledConverterApp app;
    return app.Run(argc, argv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeName)
    {
    BeAssert(0 == BentleyApi::BeStringUtilities::Wcsicmp(bridgeName, L"TiledDgnV8Bridge"));
    return new TiledConverterApp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridge_getAffinity(iModelBridge::BridgeAffinity& bridgeAffinity, BentleyApi::BeFileName const& thisLibraryPath, BentleyApi::BeFileName const& sourceFileName)
    {
    // I cannot tell if a given V8 file is part of a tiled file set or not. So, I cannot report any affinity to V8 files.
    // The bridge fwk must some other means, perhaps direct user input, to determine when to use a tiled file bridge instead of a root model bridge.
    bridgeAffinity.m_affinity = BentleyApi::Dgn::iModelBridge::Affinity::None;
    }
