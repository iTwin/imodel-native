/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/RealityContextTest/RealityContextTest.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__
#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityAdmin/ContextServicesWorkbench.h>
#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>


//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     8/2016
//=====================================================================================
enum class OperationType
    {
    SPATIAL,
    PACKID,
    PACKFILE,
    FILES
    };

struct Region
    {
public:
    float m_longitude, m_latitude, m_radius;
    Utf8String m_name;

    Region():m_longitude(0), m_latitude(0), m_radius(0), m_name(""){}
    Region(float lon, float lat, float rad, Utf8String name) : m_longitude(lon), m_latitude(lat), m_radius(rad), m_name(name) {}
    };

struct UserManager;

struct Stat
    {
public:
    int success, failure;
    uint64_t minTime, maxTime, avgTime, startTime;

    Stat() : success(0), failure(0), minTime(1000), maxTime(0), avgTime(0), startTime(std::time(nullptr)) {}
    void Update(bool success, uint64_t time);
    };

struct Stats
    {
    bmap<OperationType, Stat*> opStats;
    bvector<Utf8String>        errors;
    int                        m_activeUsers = 1;

    Stats();
    void InsertStats(OperationType type, bool success, uint64_t time, int activeUsers, Utf8String errorMsg = "");
    void PrintStats();
    void WriteToFile(int userCount, Utf8String path);
    };

struct RPS
    {
    bmap<OperationType, bmap<uint64_t, int>> requestLog;

    RPS();
    void AddRequest(OperationType type, uint64_t time);
    double GetRPS(OperationType type, uint64_t time);
    };

struct User
    {
public:
    RealityPlatform::ContextServicesWorkbench*   m_bench;
    Region                      m_region;
    Utf8StringP                 m_token;
    uint64_t                     m_downloadStart;
    OperationType               m_currentOperation;
    FILE*                       m_packageFile;
    int                         m_retryCounter;
    std::default_random_engine* m_generator;
    std::uniform_real_distribution<double>* m_distribution;
    bvector<Utf8String>         m_selectedIds;
    Utf8String                  m_packageParameters;
    RealityPlatform::CONNECTServerType m_serverType;

    User(Utf8StringP token, std::default_random_engine* generator, std::uniform_real_distribution<double>* distribution, RealityPlatform::CONNECTServerType serverType = RealityPlatform::CONNECTServerType::QA);
    ~User();
    void SelectRegion();
    void SelectExtents();
    void DoNext(UserManager* owner);
    bool ValidatePrevious(int activeUsers);
    bool ValidateSpatial(int activeUsers);
    bool ValidatePackageId(int activeUsers);
    bool ValidatePacakgeFile(int activeUsers);
    float Degree2Radians(float degree);
    float Radians2Degree(float radius);
    void SampleIds(Json::Value regionItems);
    };

struct UserManager
    {
public:
    void*                       m_pCurlHandle;
    bvector<User*>              users;
    WString                     m_certPath;
    std::default_random_engine  m_generator;
    std::uniform_real_distribution<double> m_distribution;
    Stats                       m_stats;
    int                         m_userCount;

    UserManager();
    ~UserManager();
    void Perform();
    void SetupCurl(User* bench, Utf8StringCR url, Utf8StringCP retString = nullptr, FILE* fp = nullptr, Utf8StringCR postFields = Utf8String());
    void Repopulate();
    };
