/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/FakeServer/FakeServer.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/BeFileName.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>

#undef FAKESERVER_EXPORT
#ifdef __FAKESERVER_DLL_BUILD__
    #define FAKESERVER_EXPORT EXPORT_ATTRIBUTE
#else
    #define FAKESERVER_EXPORT IMPORT_ATTRIBUTE
#endif

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN

//This class has methods that are faking each possible Rest Call to iModelHub
class FakeServer 
    {
    public:

        //!!! Generic Operations related to iModel

        //create an iModel from an already existing seed file
        //@iModelName  indicates path for seed file
        FAKESERVER_EXPORT static BeFileNameStatus CreateiModelFromSeed(WCharCP seedFilePath, WCharCP serverPath);
        FAKESERVER_EXPORT static BeFileNameStatus CreateiModel(BeFileName serverPath, WCharCP seedFile);

        //delete an existing iModel from server
        //@iModelName  name of the imodel, to be deleted
        FAKESERVER_EXPORT static BeFileNameStatus DeleteiModel(Utf8String serverPath, Utf8String filename);
        //delete all iModels from server
        FAKESERVER_EXPORT static BeFileNameStatus DeleteAlliModels(WCharCP serverPath);
        //create an instance of iModel before acquiring it
        FAKESERVER_EXPORT BeFileNameStatus CreateiModelInstance();
        //Dowload the iModel instance in order to acquire it
        FAKESERVER_EXPORT static BeFileNameStatus DownloadiModel(BeFileName downloadPath, CharCP serverPath, CharCP fileToDownload);
        //acquire an instance of an existing iModel
        FAKESERVER_EXPORT BeFileNameStatus AcquireiModel();
        FAKESERVER_EXPORT BeFileNameStatus CreateChangeSet();

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
        FAKESERVER_EXPORT GetFileInitStatus GetFileInitializationStatus();
        FAKESERVER_EXPORT bool IsFileCreated();

        FAKESERVER_EXPORT static DgnDbPtr AcquireBriefcase(DbResult &res, WCharCP filePath, WCharP file);

        //Pull ChangeSet
        FAKESERVER_EXPORT void PushChangeSet();
        FAKESERVER_EXPORT bool PushSuceeded();
        //Push ChangeSet
        //!!Locks
        //Query all locks
        FAKESERVER_EXPORT void QueryAllLocks();
        //Codes
    };