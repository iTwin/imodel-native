/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/3CSLoadTester/3CSLoadTest.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__
#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/RealityDataService.h>
#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/BeFile.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     4/2017
//=====================================================================================
enum OperationType
    {
    LIST_PROJECT,
    ADD_PROJECT,
    DELETE_PROJECT,
    GET_PROJECT,
    SAS_URI,
    LIST_CLUSTERS,
    CREATE_JOB,
    ADD_JOB,
    DELETE_JOB,
    GET_JOBS,
    GET_JOB,
    JOB_RESULT,
    JOB_CANCEL,
    last
    };

enum requestType
    {
    POST,
    GET,
    DEL
    };

struct RawRequest
    {
public:
    Utf8String url;
    bvector<Utf8String> headers;
    Utf8String payload;
    requestType type;
    };

struct FullInfo
    {
public:
    RawRequest req;
    RawServerResponse response;
    size_t id;

    Utf8String LogError() const;
    void Clear();
    };

struct UserManager;

struct Stat
    {
public:
    int success, failure;
    time_t minTime, maxTime, avgTime, startTime;

    Stat() : success(0), failure(0), minTime(1000), maxTime(0), avgTime(0), startTime(std::time(nullptr)) {}
    void Update(bool success, time_t time);
    };

struct User;

struct Stats
    {
    bmap<OperationType, Stat*> opStats;
    bvector<Utf8String>           opLog;
    bmap<OperationType, bvector<Utf8String>>        errors;
    int                        m_activeUsers = 1;

    Stats();
    void InsertStats(const User* user, bool success, int activeUsers);
    size_t LogRequest(Utf8String req);
    void PrintStats();
    void WriteToFile(int userCount, Utf8String path);
    };

struct RPS
    {
    bmap<OperationType, bmap<time_t, int>> requestLog;

    RPS();
    void AddRequest(OperationType type, time_t time);
    double GetRPS(OperationType type, time_t time);
    };

struct User
    {
public:
    std::time_t                 m_start;
    OperationType               m_currentOperation;
    Utf8String                  m_id;
    Utf8String                  m_jobId;
    bool                        m_linked;
    FullInfo                    m_correspondance;
    AzureHandshake              m_handshake;
    BeFile                      m_file;
    BeFileName                  m_fileName;
    Utf8String                  m_token;

    int                         m_userId;

    User(int id, Utf8String token);
    
    void DoNext(UserManager* owner);
    
    void ValidatePrevious(int activeUsers);

    CURL* ListProject();
    CURL* AddProject();
    CURL* DeleteProject();
    CURL* GetProject();
    CURL* SASUri();
    CURL* ListClusters();
    CURL* CreateJob();
    CURL* AddJobForId();
    CURL* DeleteJob();
    CURL* GetJobs();
    CURL* GetJobById();
    CURL* GetJobResult();
    CURL* CancelJob();

    CURL* PrepareRequest();

    void WrapUp(UserManager* owner);
    };

struct UserManager
    {
public:
    void*                       m_pCurlHandle;
    bvector<User*>              users;
    WString                     m_certPath;
    Stats                       m_stats;
    int                         m_userCount;
    Utf8String                  m_token;

    UserManager();
    ~UserManager();
    void Perform();
    void SetupCurl(CURL* curl, User* user);
    void Repopulate();
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE