/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/RealityContextTest/RealityContextTest.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

struct WorkOverlord;

struct Stat
{
public:
    int success, failure;
    time_t minTime, maxTime, avgTime, startTime;

    Stat() : success(0), failure(0), minTime(1000), maxTime(0), avgTime(0), startTime(std::time(nullptr)) {}
    void Update(bool success, time_t time);
};

struct Stats
{
    bmap<OperationType, Stat*> opStats;

    Stats();
    void InsertStats(OperationType type, bool success, time_t time);
    void PrintStats();
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
    RealityPlatform::ContextServicesWorkbench*   m_bench;
    Region                      m_region;
    Utf8StringP                 m_token;
    std::time_t                 m_downloadStart;
    OperationType               m_currentOperation;
    FILE*                       m_packageFile;
    int                         m_retryCounter;
    std::default_random_engine* m_generator;
    std::uniform_real_distribution<double>* m_distribution;
    bvector<Utf8String>         m_selectedIds;
    Utf8String                  m_packageParameters;

    User(Utf8StringP token, std::default_random_engine* generator, std::uniform_real_distribution<double>* distribution);
    ~User();
    void SelectRegion();
    void SelectExtents();
    void DoNext(WorkOverlord* owner);
    bool ValidatePrevious();
    bool ValidateSpatial();
    bool ValidatePackageId();
    bool ValidatePacakgeFile();
    float Degree2Radians(float degree);
    float Radians2Degree(float radius);
    void SampleIds(Json::Value regionItems);
};

struct WorkOverlord
{
public:
    void*                       m_pCurlHandle;
    bvector<User*>              users;
    WString                     m_certPath;
    std::default_random_engine  m_generator;
    std::uniform_real_distribution<double> m_distribution;
    Stats                       m_stats;

    WorkOverlord();
    ~WorkOverlord();
    void Perform();
    void SetupCurl(User* bench, Utf8StringCR url, Utf8StringCP retString = nullptr, FILE* fp = nullptr, Utf8StringCR postFields = Utf8String());
    void Repopulate();
};
