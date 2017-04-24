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
    LIST,
    NAVNODE,
    STAT,
    DETAILS,
    AZURE_ADDRESS,
    CREATE,
    CHANGE,
    LINK,
    UNLINK,
    REMOVE,
    DOWNLOAD,
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
    bool                        m_linked;
    NavNode*                    m_node;
    FullInfo                    m_correspondance;
    AzureHandshake              m_handshake;
    BeFile                      m_file;
    BeFileName                  m_fileName;

    int                         m_userId;

    User();
    User(int id);
    ~User();
    
    void DoNext(UserManager* owner);
    
    void ValidatePrevious(int activeUsers);

    CURL* List();
    void ValidateList(int activeUsers);
    CURL* NavNodeFunc();
    void ValidateNavNode(int activeUsers);
    CURL* Stat();
    void ValidateStat(int activeUsers);
    CURL* Details();
    void ValidateDetails(int activeUsers);
    CURL* AzureAddress();
    void ValidateAzureAddress(int activeUsers);
    CURL* Create();
    void ValidateCreate(int activeUsers);
    CURL* Change();
    void ValidateChange(int activeUsers);
    CURL* Link();
    void ValidateLink(int activeUsers);
    CURL* Unlink();
    void ValidateUnlink(int activeUsers);
    CURL* Delete();
    void ValidateDelete(int activeUsers);
    CURL* Download();
    void ValidateDownload(int activeUsers);

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