/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/iModelBridgeFwk.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iModelBridge/iModelBridge.h>
#include <iModelBridge/iModelBridgeFwkTypes.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <Logging/bentleylogging.h>
#include <WebServices/iModelHub/Client/ClientHelper.h>
#include <iModelDmsSupport/iModelDmsSupport.h>
#include <WebServices/Connect/IConnectTokenProvider.h>

BEGIN_BENTLEY_LOGGING_NAMESPACE
namespace Provider //Forward declaration for logging provider;
    {
    class Log4cxxProvider;
    }
END_BENTLEY_LOGGING_NAMESPACE

DGNPLATFORM_REF_COUNTED_PTR(IBriefcaseManagerForBridges)

BEGIN_BENTLEY_DGN_NAMESPACE

struct IModelClientForBridges;
struct iModelBridgeCallOpenCloseFunctions;

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
    friend struct IBriefcaseManagerForBridges;

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
    BentleyStatus IModelHub_DoCreatedLocalDb();
    BentleyStatus IModelHub_DoCreatedRepository();
    BentleyStatus IModelHub_DoNewBriefcaseNeedsLocks();
    BentleyStatus BootstrapBriefcase(bool& createdNewRepo);
    BentleyStatus GetSchemaLock();
    BentleyStatus ImportDgnProvenance(bool& madeChanges);
    BentleyStatus ImportElementAspectSchema(bool& madeChanges);
    
    enum class SyncState
        {
        Initial = 0,                // Initial state: we may or may not have server revisions to pull or local Txns to push
        Pushed = 1                  // PullMergePush succeeded. We still hold all locks.
        // After we release our shared locks, we return to Initial
        };

    void SetSyncState(SyncState);
    SyncState GetSyncState();

    BeSQLite::DbResult SaveBriefcaseId();
    //void SaveParentRevisionId();

    static void DecryptCredentials(Http::Credentials& credentials);
    static WString getArgValueW(WCharCP arg);
    static Utf8String getArgValue(WCharCP arg);
    //! The command-line arguments required by the iModelBridgeFwk itself that define the Job
    struct JobDefArgs
        {
        bool       m_skipAssignmentCheck = false;
        bool       m_createRepositoryIfNecessary = false;
        bool       m_storeElementIdsInBIM {};
        bool       m_mergeDefinitions = true;
        int m_maxWaitForMutex = 60000;
        Utf8String m_revisionComment;
        WString    m_bridgeRegSubKey;
        BeFileName m_bridgeLibraryName;
        BeFileName m_bridgeAssetsDir;
        BeFileName m_loggingConfigFileName;
        BeFileName m_stagingDir;
        BeFileName m_inputFileName;
        Utf8String m_jobRunCorrelationId;
        Utf8String m_jobRequestId;
        Utf8String m_jobSubjectName;
        bvector<BeFileName> m_drawingAndSheetFiles;
        BeFileName m_fwkAssetsDir;
        Json::Value m_argsJson; // additional arguments, in JSON format. Some of these may be intended for the bridge.
        bvector<WString> m_bargs;
        
        IMODEL_BRIDGE_FWK_EXPORT JobDefArgs();

        //! Parse the command-line arguments required by the iModelBridgeFwk itself, and return a vector of pointers to the remaining
        //! arguments (which are presumably the arguments to the bridge).
        BentleyStatus ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[]);

        //! Validate that all require arguments were supplied and are valid
        BentleyStatus Validate(int argc, WCharCP argv[]);

        //! Print a message describing the framework command-line arguments
        static void PrintUsage();

        //! Load the bridge library and resolve its T_iModelBridge_getInstance function.
        T_iModelBridge_getInstance* LoadBridge();

        T_iModelBridge_releaseInstance* ReleaseBridge();

        };

    //! The command-line arguments required by the iModelBridgeFwk that pertain to iModelBanks
    struct IModelBankArgs
        {
        bool m_parsedAny {};
        bool m_dmsCredentialsEncrypted{};
        uint8_t m_maxRetryCount = 3; //!< The number of times to retry a failed pull, merge, and/or push. (0 means that the framework will try operations only once and will not re-try them in case of failure.)
        Utf8String m_url;            //!< Where the iModelBank server is 
        Utf8String m_iModelId;       //!< The GUID of the iModel that the bank serves. This is used to name to local briefcase and as a means of checking that the URL is correct.
        Utf8String m_accessToken;    //!< The token that identifies the user and the user's rights in this environment. (Is passed in http headers as the authorization property.)
        bvector<WString> m_bargs;

        BentleyStatus ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[]);
        BentleyStatus Validate(int argc, WCharCP argv[]);
        static void PrintUsage();
        bool IsDefined() const {return !m_url.empty();}
        bool ParsedAny() const {return m_parsedAny;}
        Utf8String GetBriefcaseBasename() const;
        };

    //! The command-line arguments required by the iModelBridgeFwk that pertain to the iModelHub
    struct IModelHubArgs
        {
        bool m_parsedAny {};
        bool                m_haveProjectGuid {}; //!< Was a project GUID supplied? If so, m_bcsProjectId is the GUID. Else, assume m_bcsProjectId is the project name.
        bool                m_isEncrypted;        //!< Are credentials encrypted?
        Utf8String          m_bcsProjectId;       //!< iModelHub project 
        Utf8String          m_repositoryName;     //!< A repository in the iModelHub project
        Http::Credentials   m_credentials;        //!< User credentials
        WebServices::UrlProvider::Environment m_environment;    //!< Connect environment
        uint8_t             m_maxRetryCount = 3;  //! The number of times to retry a failed pull, merge, and/or push. (0 means that the framework will try operations only once and will not re-try them in case of failure.)
        bvector<WString>    m_bargs;
        
        BentleyStatus ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[]);
        BentleyStatus Validate(int argc, WCharCP argv[]);
        static void PrintUsage();
        bool ParsedAny() const {return m_parsedAny;}
        WebServices::IConnectTokenProviderPtr m_tokenProvider;
        };

    struct DmsServerArgs
        {
        WString             m_inputFileUrn;
        BeFileName          m_workspaceDir;
        BeFileName          m_dmsLibraryName;
        Http::Credentials   m_dmsCredentials;            //!< DMS credentials
        WString             m_dataSource;
        int                 m_folderId;
        int                 m_documentId;
        int                 m_maxRetryCount;
        bool                m_isv8i;
        bvector<WString>    m_bargs;
        BeFileName          m_applicationWorkspace;
        static void PrintUsage();

        DmsServerArgs();

        T_iModelDmsSupport_getInstance*   LoadDmsLibrary();
        //void*   ReleaseDmsLibrary();

        //! Parse the command-line arguments required by the iModelBridgeFwk itself, and return a vector of pointers to the remaining
        //! arguments (which are presumably the arguments to the bridge).
        BentleyStatus ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[], bool isEncrypted);

        //! Validate that all require arguments were supplied and are valid
        BentleyStatus Validate(int argc, WCharCP argv[]);

        void SetDgnArg(WString argName, WStringCR arg, bvector<WCharCP>& bargptrs);
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
    IModelClientForBridges* m_client;
    EffectiveServerError m_lastServerError;
    iModelBridge::IBriefcaseManager::PushStatus m_lastBridgePushStatus;
    bvector<DgnModelId> m_modelsInserted;
    iModelBridge* m_bridge;
    bvector<WCharCP> m_bargptrs;        // bridge command-line arguments
    JobDefArgs m_jobEnvArgs;            // the framework's command-line arguments
    struct {
        bool m_useIModelHub;
        union {
            IModelHubArgs* m_iModelHubArgs;
            IModelBankArgs* m_iModelBankArgs;
            };
        
        };
    Utf8String m_briefcaseBasename;
    int m_maxRetryCount;
    bool m_isCreatingNewRepo {};
    DmsServerArgs m_dmsServerArgs;
    FwkRepoAdmin* m_repoAdmin {};
    IDmsSupport*    m_dmsSupport;
    NativeLogging::Provider::Log4cxxProvider* m_logProvider;
    IBriefcaseManagerForBridgesPtr m_bcMgrForBridges;

    BeSQLite::DbResult OpenOrCreateStateDb();
    void PrintUsage(WCharCP programName);
    void RedirectStderr();
    void LogStderr();
    void CleanJobWorkdir();
    void InitLogging();

    bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override;
    BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties&, BeFileNameCR fn) override;
    BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid) override;
    BentleyStatus _AssignFileToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override;
    IModelBridgeRegistry& GetRegistry();

    DgnProgressMeter& GetProgressMeter() const;

    //! @name sync with server
    //! @{
    BentleyStatus Briefcase_Initialize(int argc, WCharCP argv[]);
    bool Briefcase_IsInitialized() const {return nullptr != m_client;}
    void Briefcase_PrintUsage();
    void Briefcase_Shutdown();
    bool Briefcase_IsBriefcase();
    BentleyStatus Briefcase_CreateRepository0(BeFileNameCR localDb);
    BentleyStatus Briefcase_IModelHub_CreateRepository();
    void Briefcase_MakeBriefcaseName(); // Sets m_outputName
    BentleyStatus Briefcase_AcquireBriefcase();
    BentleyStatus Briefcase_AcquireExclusiveLocks();
    BentleyStatus Briefcase_Push(Utf8CP);
    BentleyStatus Briefcase_PullMergePush(Utf8CP, bool doPullAndMerge = true, bool doPush = true);
    BentleyStatus Briefcase_ReleaseAllPublicLocks();
    //! @}

    BentleyStatus ParseDocProps();

    void GetMutexName(wchar_t* buf, size_t bufLen);
    int RunExclusive(int argc, WCharCP argv[]);
    BentleyStatus  TryOpenBimWithBisSchemaUpgrade();
    int UpdateExistingBim();
    int UpdateExistingBimWithExceptionHandling();
    int MakeSchemaChanges(iModelBridgeCallOpenCloseFunctions&);
    int MakeDefinitionChanges(SubjectCPtr& jobsubj, iModelBridgeCallOpenCloseFunctions&);
    void OnUnhandledException(Utf8CP);
    Utf8String GetRevisionComment();
    void SetBridgeParams(iModelBridge::Params&, FwkRepoAdmin*);
    BentleyStatus ReleaseBridge();
    BentleyStatus LoadBridge();
    BentleyStatus InitBridge();

    BentleyStatus LoadDmsLibrary();
    BentleyStatus ReleaseDmsLibrary();
    BentleyStatus StageInputFile();
    BentleyStatus StageWorkspace();
    BentleyStatus SetupDmsFiles();
    int PullMergeAndPushChange(Utf8StringCR description, bool releaseLocks);
    int StoreHeaderInformation();
    
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
    IMODEL_BRIDGE_FWK_EXPORT static BeFileName ComputeReportFileName(BeFileNameCR bcName);
    //! @private
    IMODEL_BRIDGE_FWK_EXPORT void ReportIssue(WStringCR);
    //! @private
    void ReportIssue(Utf8StringCR msg) {ReportIssue(WString(msg.c_str(), true));}
    //! @private
    IMODEL_BRIDGE_FWK_EXPORT static void* GetBridgeFunction(BeFileNameCR bridgeDllName, Utf8CP funcName);
    //! @private
    IMODEL_BRIDGE_FWK_EXPORT static void SetIModelClientForBridgesForTesting(IModelClientForBridges&);
    //! @private
    IMODEL_BRIDGE_FWK_EXPORT static void SetBridgeForTesting(iModelBridge&);
    //! @private
    IMODEL_BRIDGE_FWK_EXPORT static void SetRegistryForTesting(IModelBridgeRegistry&);

    IMODEL_BRIDGE_FWK_EXPORT void SetTokenProvider(WebServices::IConnectTokenProviderPtr provider);

    IRepositoryManagerP GetRepositoryManager(DgnDbR db) const;

    //!Internal function.
    static void LogPerformance(StopWatch& stopWatch, Utf8CP description, ...)
        {
        stopWatch.Stop();
        const NativeLogging::SEVERITY severity = NativeLogging::LOG_INFO;
        NativeLogging::ILogger* logger = NativeLogging::LoggingManager::GetLogger("iModelBridge.Performance");
        if (NULL == logger)
            return;

        if (logger->isSeverityEnabled(severity))
            {
            va_list args;
            va_start(args, description);
            Utf8String formattedDescription;
            formattedDescription.VSprintf(description, args);
            va_end(args);

            logger->messagev(severity, "%s|%.0f millisecs", formattedDescription.c_str(), stopWatch.GetElapsedSeconds() * 1000.0);
            }
        }
};

END_BENTLEY_DGN_NAMESPACE