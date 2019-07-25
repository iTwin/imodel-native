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

#include <Licensing/SaasClient.h>
#include <Licensing/Utils/FeatureEvent.h>
#include <Licensing/AuthType.h>
#include <Licensing/UsageType.h>

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
