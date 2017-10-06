#pragma once
#include <Bentley/BeFileName.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN
//This class has methods that are faking each possible Rest Call to iModelHub
class CentralRepository 
    {
    public:
        static void GrantLock();
        static void EntertainPullRequest();
        static void EntertainPushRequest();
    };

class FakeServer 
    {
    public:
        //@path  indicates path where our fake server would exists
        static BeFileNameStatus CreateFakeServer(WCharCP path);
        //!!!
        //!!! Generic Operations related to iModel

        //create an iModel from an already existing seed file
        //@iModelName  indicates path for seed file
        static BeFileNameStatus CreateiModelFromSeed(WCharCP seedFilePath, WCharCP serverPath, WCharP seedFile);
        static BeFileNameStatus CreateiModel(WCharCP serverPath, WCharP seedFile);

        //delete an existing iModel from server
        //@iModelName  name of the imodel, to be deleted
        static BeFileNameStatus DeleteiModel(WCharCP serverPath, WCharCP filename);
        //delete all iModels from server
        static BeFileNameStatus DeleteAlliModels(WCharCP serverPath);
        //create an instance of iModel before acquiring it
        BeFileNameStatus CreateiModelInstance();
        //Dowload the iModel instance in order to acquire it
        static BeFileNameStatus DownloadiModel(WCharCP downloadPath, WCharCP serverPath, WCharP fileToDownload);
        //acquire an instance of an existing iModel
        BeFileNameStatus AcquireiModel();


        //!!!
        //!!! Operations related to an instance of iModel
        enum GetFileInitStatus
            {
            Successful = 0,
            NotStarted = 1,
            Scheduled = 2,
            Failed = 3,
            OutdatedFile = 4,
            IncorrectFileId = 5
            };
        //Send get request to find out file initialization status
        GetFileInitStatus GetFileInitializationStatus();
        bool IsFileCreated();

        static DgnDbPtr AcquireBriefcase(DbResult &res, WCharCP filePath, WCharP file);

        //Pull ChangeSet
        void PushChangeSet();
        bool PushSuceeded();
        //Push ChangeSet
        //!!Locks
        //Query all locks
        void QueryAllLocks();
        //Codes
    };