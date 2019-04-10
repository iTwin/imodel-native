/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/AccessKeyClientImpl.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ClientImpl.h"


BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct AccessKeyClientImpl> AccessKeyClientImplPtr;

struct AccessKeyClientImpl : ClientImpl
    {
protected:
    Utf8String m_accessKey;
    bool m_isAccessKeyValid;
    bool ValidateAccessKey();
    std::shared_ptr<Policy> GetPolicyToken();
    std::list<std::shared_ptr<Policy>> GetUserPolicies();

    void PolicyHeartbeat(int64_t currentTime);

public:
    LICENSING_EXPORT AccessKeyClientImpl
        (
        Utf8StringCR accessKey,
        WebServices::ClientInfoPtr clientInfo,
        BeFileNameCR db_path,
        bool offlineMode,
        IPolicyProviderPtr policyProvider,
        IUlasProviderPtr ulasProvider,
        Utf8StringCR projectId,
        Utf8StringCR featureString,
        ILicensingDbPtr licensingDb
        );
    LICENSING_EXPORT LicenseStatus StartApplication();
    LICENSING_EXPORT BentleyStatus StopApplication();

    LICENSING_EXPORT LicenseStatus GetLicenseStatus();

    LICENSING_EXPORT void DeleteAllOtherPoliciesByKey(std::shared_ptr<Policy> policy);
    };

END_BENTLEY_LICENSING_NAMESPACE
