/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/iModelBridgeFwk.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#ifdef __IMODEL_BRIDGE_FWK_BUILD__
    #define IMODEL_BRIDGE_FWK_EXPORT EXPORT_ATTRIBUTE
#else
    #define IMODEL_BRIDGE_FWK_EXPORT IMPORT_ATTRIBUTE
#endif


#include <iModelBridge/iModelBridge.h>
#include <iModelBridge/iModelBridgeFwkTypes.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <Logging/bentleylogging.h>
#include <WebServices/iModelHub/Client/ClientHelper.h>

BEGIN_BENTLEY_DGN_NAMESPACE

struct iModelHubFX;

//=======================================================================================
// @bsiclass
//=======================================================================================
// TRICKY: Must use BENTLEY_TRANSLATABLE_STRINGS macros, and NOT IMODELBRIDGEFX_TRANSLATABLE_STRINGS macros. That is because
// the fwk plays the role of the platform here. The bridge is the "app". The bridge-specific L10 db is layered
// on top of the base BeSQLite::L10N db.
BENTLEY_TRANSLATABLE_STRINGS_START(iModelBridgeFwkErrors, iModelBridgeFwkErrors)
    L10N_STRING(STATUS_TBD)                    // =="TBD"==
BENTLEY_TRANSLATABLE_STRINGS_END

//=======================================================================================
// @bsiclass                                                    Sam.Wilson   02/15
//=======================================================================================
struct iModelBridgeFwk : iModelBridge::IDocumentPropertiesAccessor
{
    enum class EffectiveServerError
        {
        Unknown = 1,
        iModelDoesNotExist = 2
        };

    // BootstrappingState tells us where we are in the bootstrapping process.
    //
    //                                          have BootstrappingState file?
    //                                                    |
    //                                                   no:
    //                                                    v
    //                                                  *Initial*
    //                                                have briefcase?
    //                                              /               \
    //                                            no:               yes:
    //                                      CleanJobWorkdir          |
    //                                      try AcquireBriefcase     |
    //                          repo_not_found:         success: --->+
    //                          /                                    |
    //                   Convert                                     |
    //                      v                                        |
    //                 *CreatedLocalDb*                              |
    //                   CreateRepository                            |
    //                      v                                        |
    //                 *CreatedRepository*                           |
    //                  delete localdb                               |
    //                  AcquireBriefcase                             |
    //                 *NewBriefcaseNeedsLocks*                      |
    //                  LockModelsExclusively                        |
    //                            +--------------->+<----------------+
    //                                             v
    //                                       *HaveBriefcase*
    //
    enum class BootstrappingState
        {
        Initial = 0,                // This is our initial state, when we have an empty job work dir
        CreatedLocalDb = 1,         // We reach this state after the first stage in creating a new repository
        CreatedRepository = 2,      // We reach this state after creating a new repo, but before acquiring a briefcase.
        NewBriefcaseNeedsLocks = 3, // we have a briefcase for a newly created repository. We must lock all newly created models exclusively.
        HaveBriefcase = 4,          // We have a briefcase. This is where we want to be all of the time.
        LastState = HaveBriefcase
        };

    void SaveNewModelIds();
    void ReadNewModelIds();
    void SetState(BootstrappingState);
    BootstrappingState GetState();
    BentleyStatus AssertPreConditions();
    BentleyStatus DoInitial();
    BentleyStatus DoCreatedLocalDb();
    BentleyStatus DoCreatedRepository();
    BentleyStatus DoNewBriefcaseNeedsLocks();
    BentleyStatus BootstrapBriefcase(bool& createdNewRepo);

    enum class SyncState
        {
        Initial = 0,                // Initial state: we may or may not have server revisions to pull or local Txns to push
        Pushed = 1                  // PullMergePush succeeded. We still hold all locks.
        // After we release our shared locks, we return to Initial
        };

    void SetSyncState(SyncState);
    SyncState GetSyncState();

    void SaveBriefcaseId();
    //void SaveParentRevisionId();

    //! The command-line arguments required by the iModelBridgeFwk itself that define the Job
    struct JobDefArgs
        {
        bool m_skipAssignmentCheck = false;
        bool m_createRepositoryIfNecessary = false;
        int m_maxWaitForMutex = 60000;
        Utf8String m_revisionComment;
        WString m_bridgeRegSubKey;
        BeFileName m_bridgeLibraryName;
        BeFileName  m_bridgeAssetsDir;
        BeFileName m_loggingConfigFileName;
        BeFileName m_stagingDir;
        BeFileName m_inputFileName;
        bvector<BeFileName> m_drawingAndSheetFiles;
        BeFileName m_fwkAssetsDir;
        iModelBridge::GCSDefinition m_inputGcs;
        iModelBridge::GCSCalculationMethod m_gcsCalculationMethod;
        Transform m_spatialDataTransform;
        bvector<WString> m_bargs;

        IMODEL_BRIDGE_FWK_EXPORT JobDefArgs();

        //! Parse the command-line arguments required by the iModelBridgeFwk itself, and return a vector of pointers to the remaining
        //! arguments (which are presumably the arguments to the bridge).
        BentleyStatus ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[]);

        //! Parse additional arguments in the form of JSON
        BentleyStatus ParseJsonArgs(JsonValueCR);

        //! Validate that all require arguments were supplied and are valid
        BentleyStatus Validate(int argc, WCharCP argv[]);

        //! Print a message describing the framework command-line arguments
        static void PrintUsage();

        //! Load the bridge library and resolve its T_iModelBridge_getInstance function.
        T_iModelBridge_getInstance* LoadBridge();
        };

    //! The command-line arguments required by the iModelBridgeFwk that pertain to the iModelHub
    struct ServerArgs
        {
        bool m_haveProjectGuid {};                  //!< Was a project GUID supplied? If so, m_bcsProjectId is the GUID. Else, assume m_bcsProjectId is the project name.
        Utf8String m_bcsProjectId;                  //!< iModelHub project 
        Utf8String m_repositoryName;                //!< A repository in the iModelHub project
        Http::Credentials m_credentials;            //!< User credentials
        WebServices::UrlProvider::Environment m_environment; //!< Connect environment
        uint8_t m_maxRetryCount = 2;                //! The number of times to retry a failed pull, merge, and/or push. (0 means that the framework will try operations only once and will not re-try them in case of failure.)
        bvector<WString> m_bargs;

        //! Parse the command-line arguments required by the iModelBridgeFwk itself that pertain to the iModelHub, and return a vector of pointers to the remaining
        //! arguments (which are presumably the arguments to the bridge).
        BentleyStatus ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[]);

        //! Validate that all require arguments were supplied and are valid
        BentleyStatus Validate(int argc, WCharCP argv[]);

        //! Print a message describing the framework command-line arguments
        static void PrintUsage();
        };

    //! Admin that supplies the live repository connection that the fwk has created, plus the bulk insert briefcasemgr that
    //! all bridges should use.
    struct FwkRepoAdmin : DgnPlatformLib::Host::RepositoryAdmin
        {
        iModelBridgeFwk& m_fwk;
        IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const override;
        FwkRepoAdmin(iModelBridgeFwk& fwk) : m_fwk(fwk) {}
        };

protected:
    DgnDbPtr m_briefcaseDgnDb;
    BeSQLite::Db m_stateDb;
    RefCountedPtr<IModelBridgeRegistry> m_registry;
    BeFileName m_briefcaseName;
    BeFileName m_stdoutFileName;
    BeFileName m_stderrFileName;
    iModelHubFX* m_clientUtils;
    EffectiveServerError m_lastServerError;
    bvector<DgnModelId> m_modelsInserted;
    iModelBridge* m_bridge;
    bvector<WCharCP> m_bargptrs;        // bridge command-line arguments
    JobDefArgs m_jobEnvArgs;                  // the framework's command-line arguments
    ServerArgs m_serverArgs;            // the framework's command-line arguments that pertain to the iModelHub
    FwkRepoAdmin* m_repoAdmin {};

    BeSQLite::DbResult OpenOrCreateStateDb();
    void PrintUsage(WCharCP programName);
    void RedirectStderr();
    void LogStderr();
    void CleanJobWorkdir();
    void InitLogging();

    bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override;
    BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties&, BeFileNameCR fn) override;
    BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid) override;
    IModelBridgeRegistry& GetRegistry();

    DgnProgressMeter& GetProgressMeter() const;

    //! @name sync with server
    //! @{
    BentleyStatus Briefcase_Initialize(int argc, WCharCP argv[]);
    bool Briefcase_IsInitialized() const {return nullptr != m_clientUtils;}
    BentleyStatus Briefcase_ParseCommandLine(int argc, WCharCP argv[]);
    void Briefcase_PrintUsage();
    void Briefcase_Shutdown();
    bool Briefcase_IsBriefcase();
    BentleyStatus Briefcase_CreateRepository0(BeFileNameCR localDb);
    BentleyStatus Briefcase_CreateRepository();
    void Briefcase_MakeBriefcaseName(); // Sets m_outputName
    BentleyStatus Briefcase_AcquireBriefcase();
    BentleyStatus Briefcase_AcquireExclusiveLocks();
    BentleyStatus Briefcase_PullAndMerge();
    BentleyStatus Briefcase_PullMergePush(Utf8CP);
    BentleyStatus Briefcase_ReleaseSharedLocks();
    //! @}

    BentleyStatus ParseDocProps();

    WString GetMutexName();
    int RunExclusive(int argc, WCharCP argv[]);
    int UpdateExistingBim();
    void SetBridgeParams(iModelBridge::Params&, FwkRepoAdmin*);
    BentleyStatus LoadBridge();
    BentleyStatus InitBridge();
    int ProcessSchemaChange();

public:
    IMODEL_BRIDGE_FWK_EXPORT iModelBridgeFwk();
    IMODEL_BRIDGE_FWK_EXPORT ~iModelBridgeFwk();

    //! wmain should call this first
    IMODEL_BRIDGE_FWK_EXPORT BentleyStatus ParseCommandLine(int argc, WCharCP argv[]);

    //! wmain should call this to run the bridge, connected to iModelHub.
    IMODEL_BRIDGE_FWK_EXPORT int Run(int argc, WCharCP argv[]);

    static NativeLogging::ILogger& GetLogger() { return *NativeLogging::LoggingManager::GetLogger("iModelBridge"); }
    bool GetCreateRepositoryIfNecessary() const {return m_jobEnvArgs.m_createRepositoryIfNecessary;}
    bool GetSkipAssignmentCheck() const {return m_jobEnvArgs.m_skipAssignmentCheck;}
    BeFileName GetLoggingConfigFileName() const {return m_jobEnvArgs.m_loggingConfigFileName;}
    void SetBriefcaseBim(DgnDbR db) { m_briefcaseDgnDb = &db; }
    DgnDbPtr GetBriefcaseBim() { return m_briefcaseDgnDb; }

    //! @private
    IMODEL_BRIDGE_FWK_EXPORT static void* GetBridgeFunction(BeFileNameCR bridgeDllName, Utf8CP funcName);

    //! @private
    IMODEL_BRIDGE_FWK_EXPORT static void SetiModelHubFXForTesting(iModelHubFX&);
    //! @private
    IMODEL_BRIDGE_FWK_EXPORT static void SetBridgeForTesting(iModelBridge&);
    //! @private
    IMODEL_BRIDGE_FWK_EXPORT static void SetRegistryForTesting(IModelBridgeRegistry&);

    IRepositoryManagerP GetRepositoryManager(DgnDbR db) const;
};

END_BENTLEY_DGN_NAMESPACE