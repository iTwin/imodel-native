/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridge.h>
#include <iModelBridge/iModelBridgeBimHost.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_IMODELHUB

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

namespace {
  struct HostTerminator
    {
    ~HostTerminator()
        {
        DgnPlatformLib::GetHost().Terminate(true);
        iModelBridge::L10N::Terminate();
        }
    };
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static void initLoggingForDiscloseFilesAndAffinities()
    {
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity(L"iModelBridge", NativeLogging::LOG_TRACE);
    //NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity(L"ECDb", NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(L"DgnCore", NativeLogging::LOG_WARNING);
    NativeLogging::LoggingConfig::SetSeverity(L"DgnV8Converter", NativeLogging::LOG_INFO);
    // NativeLogging::LoggingConfig::SetSeverity(L"Changeset", NativeLogging::LOG_TRACE);
    //NativeLogging::LoggingConfig::SetSeverity(L"BeSQLite", NativeLogging::LOG_TRACE);
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridge::DiscloseFilesAndAffinities(iModelBridge& bridge, WCharCP outputFileName, WCharCP affinityLibraryPathStr, WCharCP assetsPathStr, WCharCP sourceFileNameStr, WCharCP bridgeId)
    {
    initLoggingForDiscloseFilesAndAffinities();

    // Set up the imodel02 Host. The bridge expects the framework to do that.
    BeFileName assetsPath(assetsPathStr);

    BeFileName fwkDb3(assetsPath);
    fwkDb3.AppendToPath(L"sqlang");
    fwkDb3.AppendToPath(L"iModelBridgeFwk_en-US.sqlang.db3");

    static iModelBridgeBimHost s_host(new DgnPlatformLib::Host::RepositoryAdmin(), assetsPath, fwkDb3, "");
    DgnPlatformLib::Initialize(s_host);

    BeFileName bridgeSqlangPath(assetsPath);
    bridgeSqlangPath.AppendToPath(bridge._SupplySqlangRelPath().c_str());
    iModelBridge::L10N::Initialize(BeSQLite::L10N::SqlangFiles(bridgeSqlangPath));

    HostTerminator  terminateOnReturn;

    // Open the output Db, where we want the bridge to write its results. This is not an iModel.
    auto db = iModelBridgeAffinityDb::OpenOrCreate(BeFileName(outputFileName));
    if (!db.IsValid())
        return BSIERROR;

    // Prepare a dummy .bim file for the bridge to write side-effects to.
    WString tempBaseName = BeFileName::GetFileNameWithoutExtension(affinityLibraryPathStr).append(L"-").append(BeFileName::GetFileNameWithoutExtension(sourceFileNameStr));
    BeFileName dbFileName(nullptr, BeFileName::GetDirectoryName(outputFileName).c_str(), tempBaseName.c_str(), L".bim");

    BeSQLite::DbResult fileStatus;
    DgnDbPtr scratchBim;
    if (BeFileName::DoesPathExist(dbFileName))
        {
        scratchBim = DgnDb::OpenDgnDb(&fileStatus, dbFileName, DgnDb::OpenParams(BeSQLite::Db::OpenMode::ReadWrite));
        if (!scratchBim.IsValid())
            BeFileName::BeDeleteFile(dbFileName);
        }
    if (!scratchBim.IsValid())
        {
        Dgn::CreateDgnDbParams createParams("temporary briefcase for disclose references");
        createParams.SetOverwriteExisting(true);
        scratchBim = Dgn::DgnDb::CreateDgnDb(&fileStatus, dbFileName, createParams);
        if (!scratchBim.IsValid() || BeSQLite::BE_SQLITE_OK != fileStatus)
            return BSIERROR;
        }

    // Set up the bridge as if we were going to ask it to convert the input file.
    WCharCP argv[] = {L"arg0"};

    auto& params = bridge._GetParams();
    params.m_briefcaseName = dbFileName;
    params.m_thisBridgeRegSubKey = bridgeId;
    params.m_assetsDir = assetsPath;
    params.m_libraryDir.assign(BeFileName::GetDirectoryName(affinityLibraryPathStr).c_str());
    params.m_affinityLibraryPath.assign(affinityLibraryPathStr);
    params.m_inputFileName.assign(sourceFileNameStr);

    BentleyStatus res;
    if (BSISUCCESS == (res = bridge._Initialize(_countof(argv), argv))
     && BSISUCCESS == (res = bridge._OnOpenBim(*scratchBim))
     && BSISUCCESS == (res = bridge._OpenSource()))
        {
        //  First, make sure we have a JobSubject element. When initializing, this will entail reserving a Code and inserting into the RepositoryModel.
        bridge._GetParams().SetIsUpdating(true);
        auto jobsubj = bridge._FindJob();
        if (!jobsubj.IsValid())
            {
            bridge._GetParams().SetIsUpdating(false);
            scratchBim->BriefcaseManager().GetChannelPropsR().isInitializingChannel = true;
            jobsubj = bridge._InitializeJob();    // this is the first time that this bridge has tried to convert this input file into this iModel
            scratchBim->BriefcaseManager().GetChannelPropsR().isInitializingChannel = false;
            if (!jobsubj.IsValid())
                {
                LOG.fatalv("Failed to create job structure");
                return BSIERROR;
                }
            }

        bridge._GetParams().SetJobSubjectId(jobsubj->GetElementId());

        // ... and ask it to disclose references instead of converting it. 
        res = bridge._DiscloseFilesAndAffinities(*db);
        }

    scratchBim->SaveChanges();
    db->GetDb().SaveChanges();

    // Tear down the bridge. We will only use it once.
    bridge._CloseSource(res, ClosePurpose::Finished);
    bridge._OnCloseBim(res, ClosePurpose::Finished);

    bridge._Terminate(res);

    scratchBim->SaveChanges();
    scratchBim->CloseDb();
    scratchBim = nullptr;

    db->GetDb().SaveChanges();
    db->GetDb().CloseDb();
    db = nullptr;

    return res;
    }
