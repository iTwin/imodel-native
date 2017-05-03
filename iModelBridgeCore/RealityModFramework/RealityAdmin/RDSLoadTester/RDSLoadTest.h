/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/RDSLoadTester/RDSLoadTest.h $
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
    CREATE_REALITYDATA,
    CREATE_RELATIONSHIP,
    DETAILS,
    LIST_REALITYDATA,
    LIST_RELATIONSHIP,
    ENTERPRISE_STAT,
    AZURE_ADDRESS,
    MODIFY_REALITYDATA,
    DELETE_RELATIONSHIP,
    DELETE_REALITYDATA,
    last
    };

struct RawRequest
    {
public:
    Utf8String url;
    Utf8String headers;
    Utf8String payload;
    };

struct FullInfo
    {
public:
    RawRequest req;
    RawServerResponse response;
    size_t id;

    Utf8String LogError() const;
    };

struct UserManager;

struct Stat
    {
public:
    int success, failure;
    int64_t minTime, maxTime, avgTime, startTime;

    Stat() : success(0), failure(0), minTime(10000), maxTime(0), avgTime(0), startTime(std::time(nullptr)) {}
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
    int64_t                     m_start;
    int64_t                     m_lastRequestTimeMilliseconds;
    OperationType               m_currentOperation;
    Utf8String                  m_id;
    bool                        m_linked;
    FullInfo                    m_correspondance;
    AzureHandshake              m_handshake;
    BeFile                      m_file;
    BeFileName                  m_fileName;

    int                         m_userId;

    User();
    User(int id);
    
    void DoNext(UserManager* owner);
    
    void ValidatePrevious(int activeUsers);

    CURL* ListRealityData();
    void ValidateListRealityData(int activeUsers);
    CURL* ListRelationship();
    void ValidateListRelationship(int activeUsers);
    CURL* EnterpriseStat();
    void ValidateEnterpriseStat(int activeUsers);
    CURL* Details();
    void ValidateDetails(int activeUsers);
    CURL* AzureAddress();
    void ValidateAzureAddress(int activeUsers);
    CURL* CreateRealityData();
    void ValidateCreateRealityData(int activeUsers);
    CURL* ModifyRealityData();
    void ValidateModifyRealityData(int activeUsers);
    CURL* CreateRelationship();
    void ValidateCreateRelationship(int activeUsers);
    CURL* DeleteRelationship();
    void ValidateDeleteRelationship(int activeUsers);
    CURL* DeleteRealityData();
    void ValidateDeleteRealityData(int activeUsers);

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

    UserManager();
    ~UserManager();
    void Perform();
    void SetupCurl(CURL* curl, User* user);
    void Repopulate();
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE