/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__
#include "../LoadTestBase.h"

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     4/2017
//=====================================================================================
enum OperationType
    {
    CREATE_REALITYDATA = 0,
    CREATE_RELATIONSHIP = 1,
    DETAILS = 2,
    LIST_REALITYDATA = 3,
    LIST_RELATIONSHIP = 4,
    ENTERPRISE_STAT = 5,
    AZURE_ADDRESS = 6,
    MODIFY_REALITYDATA = 7,
    DELETE_RELATIONSHIP = 8,
    DELETE_REALITYDATA = 9,
    SERVICE_STAT = 10,

    last =11
    };

struct RDSRPS : RPS
    {
    RDSRPS();
    };

struct RDSStats : public Stats
    {
    RDSStats(RDSRPS* rps);
    void PrintStatsBody(int64_t currentTime, int& totalSuccess, int& totalFailure) override;
    void WriteToFileBody(int userCount, Utf8String path, std::ofstream& file) override;
    };

struct RDSUser : public User
    {
public:
    AzureHandshake              m_handshake;
    RealityData&                m_realityData;
    Utf8String                  m_fileName;

    // RDSUser();
    RDSUser(int id, Stats* stats, RealityData& realityData, Utf8String fileName);
    
    bool DoNextBody(UserManager* owner) override;
    
    void ValidatePrevious(int activeUsers) override;

    CURL* ListRealityData();
    void ValidateListRealityData(int activeUsers);
    CURL* ListRelationship();
    void ValidateListRelationship(int activeUsers);
    CURL* EnterpriseStat();
    void ValidateEnterpriseStat(int activeUsers);
    CURL* ServiceStat();
    void ValidateServiceStat(int activeUsers);	
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

    bool WrapUp(UserManager* owner) override;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE