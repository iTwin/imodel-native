/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/DgnV8/ConverterApp.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#define NULL 0

#include <DgnDbSync/DgnV8/Converter.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <Bentley/BeFileListIterator.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <ThreeMx/ThreeMxApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnView/DgnViewLib.h>
#include <BeSQLite/L10N.h>
#include <Logging/bentleylogging.h>
#include <iModelBridge/iModelBridgeBimHost.h>

#ifndef VERSIONEDv8APINATIVESUPPORT
USING_NAMESPACE_BENTLEY_DGN
#endif
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_DGNDBSYNC
USING_NAMESPACE_DGNDBSYNC_DGNV8
USING_NAMESPACE_BENTLEY_SQLITE

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"DgnV8Converter"))

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/15
//=======================================================================================
struct ConverterApp : iModelBridge
{
    struct Monitor : Converter::Monitor
        {
        bvector<DgnModelId> m_modelsInserted, m_modelsDeleted;
        void _OnModelInserted(ResolvedModelMapping const& m) {m_modelsInserted.push_back(m.GetDgnModel().GetModelId());}
        void _OnModelDelete(DgnModelR, SyncInfo::V8ModelMapping const& m) {m_modelsDeleted.push_back(m.GetModelId());}
        };

protected:
    // iModelBridge
    WString _SupplySqlangRelPath() override;
    CmdLineArgStatus _ParseCommandLineArg(int iarg, int argc, WCharCP argv[]) override;
    BentleyStatus _Initialize(int argc, WCharCP argv[]) override;
    void _DeleteSyncInfo() override;

    //  For subclasses to override:
    virtual Converter::Params& _GetConverterParams() = 0;
    virtual BeFileName _GetLoggingConfigurationFilename(WCharCP argv0);

    BentleyStatus CreateSyncInfoIfNecessary();

    WString GetCommonCommandLineOptions();

    RootModelConverter::Params m_spatialParams;
    bool                m_wasUpdateEmpty;
    RefCountedPtr<Monitor> m_monitor;

    ConverterApp() {m_wasUpdateEmpty=false;}
    void GetImportConfiguration(BeFileNameR instanceFilePath, BeFileNameCR configurationPath);
    BentleyStatus GetEnv (BeFileName& fn, WCharCP envname);
    DgnProgressMeter& GetProgressMeter() const;
public:
    BentleyStatus Run(int argc, WCharCP argv[]);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/15
//=======================================================================================
struct RootModelConverterApp : ConverterApp
{
    DEFINE_T_SUPER(ConverterApp)

protected:
    RootModelConverter::RootModelSpatialParams m_params;
    std::unique_ptr<RootModelConverter> m_converter;
    
    void DetectDrawingsDirs();

    Converter::Params& _GetConverterParams() override {return m_params;}

    // iModelBridge:
    void _PrintUsage() override;
    iModelBridge::Params& _GetParams() override {return m_params;}
    CmdLineArgStatus _ParseCommandLineArg(int iArg, int argc, WCharCP argv[]);
    BentleyStatus _ConvertToBim(Dgn::SubjectCR jobSubject) override;
    BentleyStatus _Initialize(int argc, WCharCP argv[]) override;
    Dgn::SubjectCPtr _InitializeJob() override;
    BentleyStatus _OnOpenBim(DgnDbR db) override;
    void _OnCloseBim(BentleyStatus, iModelBridge::ClosePurpose purpose) override;
    Dgn::SubjectCPtr _FindJob() override;
    BentleyStatus _MakeDefinitionChanges(SubjectCR) override;
    BentleyStatus _OpenSource() override;
    void _CloseSource(BentleyStatus , iModelBridge::ClosePurpose) override;
    BentleyStatus _DetectDeletedDocuments() override;
    BentleyStatus _MakeSchemaChanges() override;

    DgnFontCP _TryResolveFont(DgnFontCP font) override {return m_converter->TryResolveFont(font);}

public:
    RootModelConverterApp()
        {
        RootModelConverter::RootModelChoice c;
        c.m_method = RootModelConverter::RootModelChoice::Method::FromActiveViewGroup;
        m_params.SetRootModelChoice(c);
        }

    ~RootModelConverterApp() {BeAssert(nullptr == m_converter.get());}
    
};


END_DGNDBSYNC_DGNV8_NAMESPACE

extern "C"
    {
    EXPORT_ATTRIBUTE T_iModelBridge_getAffinity iModelBridge_getAffinity;
    EXPORT_ATTRIBUTE T_iModelBridge_getInstance iModelBridge_getInstance;
    EXPORT_ATTRIBUTE T_iModelBridge_releaseInstance iModelBridge_releaseInstance;
    }
