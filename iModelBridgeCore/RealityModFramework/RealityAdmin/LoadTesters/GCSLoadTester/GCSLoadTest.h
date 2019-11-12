/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__
#include <RealityAdmin/ContextServicesWorkbench.h>
#include <BeJsonCpp/BeJsonUtilities.h>

#include "../LoadTestBase.h"

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     8/2016
//=====================================================================================
enum OperationType
    {
    SPATIAL = 0,
    PACKID = 1,
    PACKFILE = 2,
    FILES = 3,
    last = 4
    };

struct Region
    {
public:
    float m_longitude, m_latitude, m_radius;
    Utf8String m_name;

    Region():m_longitude(0), m_latitude(0), m_radius(0), m_name(""){}
    Region(float lon, float lat, float rad, Utf8String name) : m_longitude(lon), m_latitude(lat), m_radius(rad), m_name(name) {}
    };

struct GCSRPS : public RPS
    {
    GCSRPS();
    };

struct GCSStats : public Stats
    {
    GCSStats(GCSRPS* rps);
    void PrintStatsBody(int64_t currentTime, int& totalSuccess, int& totalFailure) override;
    void WriteToFileBody(int userCount, Utf8String path, std::ofstream& file) override;
    };

struct GCSUser : public User
    {
public:
    RealityPlatform::ContextServicesWorkbench*   m_bench;
    Region                                       m_region;
    Utf8StringP                                  m_token;
    FILE*                                        m_packageFile;
    int                                          m_retryCounter;
    std::default_random_engine*                  m_generator;
    std::uniform_real_distribution<double>*      m_distribution;
    bvector<Utf8String>                          m_selectedIds;
    Utf8String                                   m_packageParameters;
    RealityPlatform::CONNECTServerType           m_serverType;

    GCSUser(Utf8StringP token, std::default_random_engine* generator, std::uniform_real_distribution<double>* distribution, int id, Stats* stats, RealityPlatform::CONNECTServerType serverType = RealityPlatform::CONNECTServerType::QA);
    ~GCSUser();
    void SelectRegion();
    void SelectExtents();
    bool DoNextBody(UserManager* owner) override;
    void ValidatePrevious(int activeUsers) override;
    bool WrapUp(UserManager* owner) override { return false; }
    bool ValidateSpatial(int activeUsers);
    bool ValidatePackageId(int activeUsers);
    bool ValidatePacakgeFile(int activeUsers);
    float Degree2Radians(float degree);
    float Radians2Degree(float radius);
    void SampleIds(Json::Value regionItems);
    CURL* SetupCurl(Utf8StringCR url, Utf8StringCP retString = nullptr, FILE* fp = nullptr, Utf8StringCR postFields = Utf8String());
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE