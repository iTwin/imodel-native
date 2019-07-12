/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <Bentley/Bentley.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeVersion.h>
#include <memory>
#include <BeJsonCpp/BeJsonUtilities.h>

#include <Bentley/BentleyConfig.h>

// WIP_LICENSING_FOR_MACOS
#if !defined(BENTLEYCONFIG_OS_APPLE_MACOS)
#include <Licensing/SaasClient.h>
#include <Licensing/Utils/FeatureEvent.h>
#include <Licensing/AuthType.h>
#include <Licensing/UsageType.h>
#else
namespace Licensing
{
typedef std::shared_ptr<struct SaasClient> SaasClientPtr;

struct FeatureEvent 
    {
    FeatureEvent(Utf8StringCR featureId, BeVersionCR version) {}
    FeatureEvent(Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId) {}
    };

enum class AuthType
    {
    None = 0,
    SAML = 1,
    OIDC = 2
    };

enum class UsageType
    {
    Production = 0,
    Trial = 1,
    Beta = 2,
    HomeUse = 3,
    PreActivation = 4,
    Evaluation = 5,
    Academic = 6
    };
};
#endif

USING_NAMESPACE_BENTLEY

namespace IModelJsNative
    {
    enum class Region
        {
        Dev = 103,
        Qa = 102,
        Prod = 0,
        Perf = 294
        };

     //=======================================================================================
    //! API to track usage of the application that uses iModel.js
    // @bsiclass                                           Krischan.Eberle      12/2018
    //+===============+===============+===============+===============+===============+======
    struct UlasClient final
        {
    private:
        RuntimeJsonLocalState m_localState;
        Licensing::SaasClientPtr m_client;
        // Bentley coding guideline: No need to free static non-POD objects.
        static UlasClient* s_singleton;

        UlasClient();
        ~UlasClient();

    public:
        static UlasClient& Get();

        void Initialize(Region);
        void Uninitialize();

        BentleyStatus TrackUsage(
            Utf8StringCR accessToken,
            BeVersionCR appVersion,
            Utf8StringCR projectId,
            Licensing::AuthType authType = Licensing::AuthType::OIDC,
            int productId = -1,
            Utf8StringCR deviceId = "",
            Licensing::UsageType usageType = Licensing::UsageType::Production,
            Utf8StringCR correlationId = ""
        ) const;

        BentleyStatus MarkFeature(
            Utf8StringCR accessToken, 
            Licensing::FeatureEvent featureEvent,
            Licensing::AuthType authType = Licensing::AuthType::OIDC,
            int productId = -1,
            Utf8StringCR deviceId = "",
            Licensing::UsageType usageType = Licensing::UsageType::Production,
            Utf8StringCR correlationId = ""
        ) const;
        };
    };
