/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/iModelBridgeFwk.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iModelBridge/iModelBridge.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <Logging/bentleylogging.h>
#include <WebServices/iModelHub/Client/ClientHelper.h>

BEGIN_BENTLEY_DGN_NAMESPACE

struct DgnDbServerClientUtils;

//=======================================================================================
// @bsiclass                                                    Sam.Wilson   02/15
//=======================================================================================
struct iModelBridgeFwk
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
    void DeleteNewModelIdsFile();
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
    BeFileName GetSyncStateFileName();

    //! The command-line arguments required by the iModelBridgeFwk itself that define the Job
    struct JobDefArgs
        {
        bool m_createRepositoryIfNecessary = false;
        int m_maxWaitForMutex = 60000;
        Utf8String m_revisionComment;
        BeFileName m_bridgeLibraryName;
        BeFileName m_loggingConfigFileName;
        BeFileName m_stagingDir;
        BeFileName m_inputFileName;
        BeFileName m_fwkAssetsDir;
        iModelBridge::GCSDefinition m_inputGcs;
        iModelBridge::GCSCalculationMethod m_gcsCalculationMethod;
        bvector<WString> m_bargs;

        //! Parse the command-line arguments required by the iModelBridgeFwk itself, and return a vector of pointers to the remaining
        //! arguments (which are presumably the arguments to the bridge).
        BentleyStatus ParseCommandLine(bvector<WCharCP>& bargptrs, int argc, WCharCP argv[]);

        //! Validate that all require arguments were supplied and are valid
        BentleyStatus Validate(int argc, WCharCP argv[]);

        //! Print a message describing the framework command-line arguments
        static void PrintUsage();

        //! Load the bridge library and resolve its T_iModelBridge_getInstance function.
        T_iModelBridge_getInstance LoadBridge();
        };

    //! The command-line arguments required by the iModelBridgeFwk that pertain to the iModelHub
    struct ServerArgs
        {
        Utf8String m_bcsProjectName;                //!< iModelHub project
        Utf8String m_repositoryName;                //!< A repository in the iModelHub project
        Http::Credentials m_credentials;            //!< User credentials
        WebServices::UrlProvider::Environment m_environment; //!< Connect environment
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
    BeFileName m_briefcaseName;
    BeFileName m_stdoutFileName;
    BeFileName m_stderrFileName;
    DgnDbServerClientUtils* m_clientUtils;
    EffectiveServerError m_lastServerError;
    bvector<DgnModelId> m_modelsInserted;
    iModelBridge* m_bridge;
    bvector<WCharCP> m_bargptrs;        // bridge command-line arguments
    JobDefArgs m_jobEnvArgs;                  // the framework's command-line arguments
    ServerArgs m_serverArgs;            // the framework's command-line arguments that pertain to the iModelHub
    FwkRepoAdmin m_repoAdmin;

    void PrintUsage(WCharCP programName);
    void RedirectStderr();
    void LogStderr();
    void CleanJobWorkdir();
    BeFileName GetStateFileName();
    BeFileName GetModelsFileName();
    void InitLogging();

    DgnProgressMeter& GetProgressMeter() const;

    //! @name sync with server
    //! @{
    BentleyStatus Briefcase_Initialize(int argc, WCharCP argv[]);
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

    WString GetMutexName();
    int Run0(int argc, WCharCP argv[]);
    void SetBridgeParams(iModelBridge::Params&);
    BentleyStatus LoadBridge();
    BentleyStatus InitBridge();

public:
    iModelBridgeFwk();
    ~iModelBridgeFwk();

    //! wmain should call this first
    BentleyStatus ParseCommandLine(int argc, WCharCP argv[]);

    //! wmain should call this to run the bridge, connected to iModelHub.
    int Run(int argc, WCharCP argv[]);

    static NativeLogging::ILogger& GetLogger() { return *NativeLogging::LoggingManager::GetLogger("iModelBridge"); }
    bool GetCreateRepositoryIfNecessary() const {return m_jobEnvArgs.m_createRepositoryIfNecessary;}
    BeFileName GetLoggingConfigFileName() const {return m_jobEnvArgs.m_loggingConfigFileName;}
    void SetBriefcaseBim(DgnDbR db) { m_briefcaseDgnDb = &db; }
    DgnDbPtr GetBriefcaseBim() { return m_briefcaseDgnDb; }

    IRepositoryManagerP GetRepositoryManager(DgnDbR db) const;
};

END_BENTLEY_DGN_NAMESPACE