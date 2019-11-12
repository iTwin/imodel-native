/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    Utf8String m_ultimateId;
    bool m_isAccessKeyValid;
    bool ValidateAccessKey();
    std::shared_ptr<Policy> GetPolicyToken();
    std::list<std::shared_ptr<Policy>> GetUserPolicies();

public:
    LICENSING_EXPORT AccessKeyClientImpl
        (
        Utf8StringCR accessKey,
        ApplicationInfoPtr applicationInfo,
        BeFileNameCR db_path,
        bool offlineMode,
        IPolicyProviderPtr policyProvider,
        IUlasProviderPtr ulasProvider,
        Utf8StringCR projectId,
        Utf8StringCR featureString,
        ILicensingDbPtr licensingDb,
        Utf8StringCR ultimateId
        );
    LICENSING_EXPORT LicenseStatus StartApplication();

    LICENSING_EXPORT LicenseStatus GetLicenseStatus();

    LICENSING_EXPORT void DeleteAllOtherPoliciesByKey(std::shared_ptr<Policy> policy);
    };

END_BENTLEY_LICENSING_NAMESPACE
