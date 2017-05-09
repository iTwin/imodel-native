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
    SUBMIT_JOB,
    DELETE_JOB,
    GET_JOBS,
    GET_JOB,
    //JOB_RESULT,
    CANCEL_JOB,
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
    int64_t minTime, maxTime, avgTime, startTime;

    Stat() : success(0), failure(0), minTime(10000), maxTime(0), avgTime(0) {}
    void Update(bool success, int64_t time);
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
    bmap<OperationType, bmap<int64_t, int>> requestLog;

    RPS();
    void AddRequest(OperationType type, int64_t time);
    double GetRPS(OperationType type, int64_t time);
    };

struct User
    {
public:
    int64_t                    m_start;
    OperationType               m_currentOperation;
    Utf8String                  m_id;
    Utf8String                  m_jobId;
    bool                        m_submitted;
    FullInfo                    m_correspondance;
    Utf8String                  m_token;

    int                         m_userId;

    User(int id, Utf8String token);
    
    void DoNext(UserManager* owner);
    
    void ValidatePrevious(int activeUsers);

    CURL* ListProject();
    CURL* AddProject();
    void  ValidateAddProject(int activeUsers);
    CURL* DeleteProject();
    CURL* GetProject();
    CURL* SASUri();
    CURL* ListClusters();
    CURL* CreateJob(Utf8String outputGuid);
    void  ValidateCreateJob(int activeUsers);
    CURL* SubmitJob();
    CURL* DeleteJob();
    CURL* GetJobs();
    CURL* GetJobById();
    //CURL* GetJobResult();
    CURL* CancelJob();
    void ValidateCancelJob(int activeUsers);

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