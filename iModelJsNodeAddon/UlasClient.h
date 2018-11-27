/*--------------------------------------------------------------------------------------+
|
|     $Source: IModelJsNative.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/BeFileName.h>

#include <memory>

// WIP until licensing client is a proper dependency
// #include <Licensing/Client.h>
namespace Licensing
    {
    struct Client
        {
        static std::shared_ptr<Client> CreateFree(Utf8StringCR accessToken, Utf8StringCR projectId, BeFileNameCR cacheDbPath) { return std::make_shared<Client>(); }
        void StopApplication() {}
        };

    typedef std::shared_ptr<Client> ClientPtr;
    }

USING_NAMESPACE_BENTLEY

namespace IModelJsNative
    {
    struct UlasClient final
        {
        private:
            static constexpr Utf8CP s_appName = "imodel.js";
            static constexpr Utf8CP s_appGuid = "a7983f22-0d27-4db9-bfc5-170bdc493f7a";
            // TODO: What version to pick?
            static constexpr uint16_t s_appVersionMajor = 1;
            static constexpr uint16_t s_appVersionMinor = 0;
            static constexpr uint16_t s_appVersionSub1 = 0;

            static BeFileName* s_cacheDbPath;

            Licensing::ClientPtr m_client;
            Utf8String m_accessToken;
            Utf8String m_projectId;
            // static WebServices::ClientInfoPtr GetClientInfo();
        public:
            UlasClient(Utf8StringCR accessToken, Utf8StringCR projectId) : m_accessToken(accessToken), m_projectId(projectId) {}
            BentleyStatus Initialize();
            void StopApplication();
        };
    };
