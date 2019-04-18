/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__
#include "../LoadTestBase.h"

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     4/2017
//=====================================================================================
enum OperationType
    {
    //GET
    LIST_PROJECT = 0,
    GET_PROJECT = 1,
    SAS_URI = 2,
    LIST_CLUSTERS = 3,
    GET_JOB = 4,
    GET_JOBS = 5,
    //POST
    ADD_PROJECT = 6,
    CREATE_JOB = 7,
    SUBMIT_JOB = 8,
    CANCEL_JOB = 9,
    //DELETE
    DELETE_PROJECT = 10,
    DELETE_JOB = 11,
    //JOB_RESULT,
    last = 12
    };

enum requestType
    {
    POST,
    GET,
    DEL
    };

struct ThreeCSRPS : RPS
    {
    ThreeCSRPS();
    };

struct ThreeCSStats: public Stats
    {
public:
    ThreeCSStats(ThreeCSRPS* rps);
    void PrintStatsBody(int64_t currentTime, int& totalSuccess, int& totalFailure) override;
    void WriteToFileBody(int userCount, Utf8String path, std::ofstream& file) override;
    };

struct ThreeCSUser : public User
    {
public:
    Utf8String                  m_jobId;
    bool                        m_submitted;
    Utf8String                  m_token;
    Utf8String                  m_RDGuid;

    ThreeCSUser(int id, Utf8String token, Utf8String rdGuid, Stats* stats);
    
    bool DoNextBody(UserManager* owner) override;
    
    void ValidatePrevious(int activeUsers) override;

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
    void  ValidateSubmitJob(int activeUsers);
    CURL* DeleteJob();
    CURL* GetJobs();
    CURL* GetJobById();
    //CURL* GetJobResult();
    CURL* CancelJob();
    void ValidateCancelJob(int activeUsers);

    CURL* PrepareRequest();

    bool WrapUp(UserManager* owner);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE