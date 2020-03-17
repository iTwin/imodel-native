/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <Bentley/Bentley.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeVersion.h>
#include <memory>
#include <BeJsonCpp/BeJsonUtilities.h>

#include <Bentley/BentleyConfig.h>

#include <Licensing/EntitlementResult.h>
#include <Licensing/SaasClient.h>
#include <Licensing/Utils/FeatureEvent.h>
#include <Licensing/AuthType.h>
#include <Licensing/UsageType.h>

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

        /**
         * @deprecated use PostUserUsage instead.
         */
        folly::Future<BentleyStatus> TrackUsage(
            Utf8StringCR accessToken,
            BeVersionCR appVersion,
            Utf8StringCR projectId,
            Licensing::AuthType authType = Licensing::AuthType::OIDC,
            int productId = -1,
            Utf8StringCR deviceId = "",
            Licensing::UsageType usageType = Licensing::UsageType::Production,
            Utf8StringCR correlationId = ""
        ) const;

        /**
         * @throw std::runtime_error when a parameter or configuration is invalid
         * @throw Http::HttpError if the request is rejected
         */
        folly::Future<folly::Unit> PostUserUsage(
            Utf8StringCR accessToken,
            BeVersionCR appVersion,
            Utf8StringCR projectId,
            Licensing::AuthType authType = Licensing::AuthType::OIDC,
            int productId = -1,
            Utf8StringCR deviceId = "",
            Licensing::UsageType usageType = Licensing::UsageType::Production,
            Utf8StringCR correlationId = "",
            Utf8StringCR principalId = ""
        ) const;

        /**
         * @deprecated use PostFeatureUsage instead
         */
        folly::Future<BentleyStatus> MarkFeature(
            Utf8StringCR accessToken, 
            Licensing::FeatureEvent featureEvent,
            Licensing::AuthType authType = Licensing::AuthType::OIDC,
            int productId = -1,
            Utf8StringCR deviceId = "",
            Licensing::UsageType usageType = Licensing::UsageType::Production,
            Utf8StringCR correlationId = ""
        ) const;

        /**
         * @throw std::runtime_error when a parameter or configuration is invalid
         * @throw Http::HttpError if the request is rejected
         */
        folly::Future<folly::Unit> PostFeatureUsage(
            Utf8StringCR accessToken, 
            Licensing::FeatureEvent featureEvent,
            Licensing::AuthType authType = Licensing::AuthType::OIDC,
            int productId = -1,
            Utf8StringCR deviceId = "",
            Licensing::UsageType usageType = Licensing::UsageType::Production,
            Utf8StringCR correlationId = "",
            Utf8StringCR principalId = ""
        ) const;

        BentleyStatus CheckEntitlement(
            Utf8StringCR accessToken,
            BeVersionCR appVersion,
            Utf8StringCR projectId,
            Licensing::AuthType authType,
            int productId,
            Utf8StringCR deviceId,
            Utf8StringCR correlationId,
            Licensing::EntitlementResult &entitlementResult
        ) const;
        };
    };
