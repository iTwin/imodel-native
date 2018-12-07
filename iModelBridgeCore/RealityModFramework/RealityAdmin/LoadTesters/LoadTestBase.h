/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/LoadTesters/LoadTestBase.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__
#include <queue>

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/BeFile.h>
#include <curl/curl.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     4/2017
//=====================================================================================
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

struct RPS
    {
    bmap<int, bmap<int64_t, int>> requestLog;

    REALITYDATAPLATFORM_EXPORT RPS();
    REALITYDATAPLATFORM_EXPORT void AddRequest(int type, int64_t time);
    REALITYDATAPLATFORM_EXPORT double GetRPS(int type, int64_t time);
    };

struct Stats
    {
    bmap<int, Stat*>                opStats;
    bvector<Utf8String>             opLog;
    bmap<int, bvector<Utf8String>>  errors;
    int                             m_activeUsers = 1;
    RPS*                            m_rps;

    REALITYDATAPLATFORM_EXPORT Stats(RPS* rps);
    REALITYDATAPLATFORM_EXPORT virtual void InsertStats(const User* user, bool success, int activeUsers);
    REALITYDATAPLATFORM_EXPORT size_t LogRequest(Utf8String req);
    REALITYDATAPLATFORM_EXPORT void PrintStats();
    REALITYDATAPLATFORM_EXPORT virtual void PrintStatsBody(int64_t currentTime, int& totalSuccess, int& totalFailure) = 0;
    REALITYDATAPLATFORM_EXPORT void WriteToFile(int userCount, Utf8String path);
    REALITYDATAPLATFORM_EXPORT virtual void WriteToFileBody(int userCount, Utf8String path, std::ofstream& file) = 0;
    };

struct User
    {
public:
    int64_t                     m_start;
    int64_t                     m_lastRequestTimeMilliseconds;
    int                         m_currentOperation;
    Utf8String                  m_id;
    bool                        m_linked;
    FullInfo                    m_correspondance;
    BeFile                      m_file;
    BeFileName                  m_fileName;
    bool                        m_wrappedUp;

    int                         m_userId;
    Stats*                      m_stats;

    REALITYDATAPLATFORM_EXPORT User();
    REALITYDATAPLATFORM_EXPORT User(int id, Stats* stats);

    REALITYDATAPLATFORM_EXPORT virtual bool DoNext(UserManager* owner);
    REALITYDATAPLATFORM_EXPORT virtual bool DoNextBody(UserManager* owner) = 0;
    REALITYDATAPLATFORM_EXPORT virtual void ValidatePrevious(int activeUsers) = 0;
    REALITYDATAPLATFORM_EXPORT virtual bool WrapUp(UserManager* owner) = 0;
    };

struct UserManager
    {
public:
    void*                       m_pCurlHandle;
    bvector<User*>              users;
    WString                     m_certPath;
    int                         m_userCount;

    REALITYDATAPLATFORM_EXPORT UserManager();
    REALITYDATAPLATFORM_EXPORT ~UserManager();
    REALITYDATAPLATFORM_EXPORT void Perform();
    REALITYDATAPLATFORM_EXPORT void SetupCurl(CURL* curl, User* user);
    REALITYDATAPLATFORM_EXPORT void Repopulate();
    };

struct LoadTester
    {
    RealityPlatform::CONNECTServerType m_serverType;
    UserManager userManager;
    bool trickle = false;
    Utf8String path;

    REALITYDATAPLATFORM_EXPORT LoadTester();
    REALITYDATAPLATFORM_EXPORT bool Main(int argc, char* argv[]);
    REALITYDATAPLATFORM_EXPORT void Main2(int requestBuffer);

    REALITYDATAPLATFORM_EXPORT static void SetupStaticClasses(Stats* stats, RPS* rps);
    REALITYDATAPLATFORM_EXPORT static void SetupInactiveUsers(std::queue<User*>& inactiveUsers);
    REALITYDATAPLATFORM_EXPORT static Utf8String MakeBuddiCall(WString service, int region = 0);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE