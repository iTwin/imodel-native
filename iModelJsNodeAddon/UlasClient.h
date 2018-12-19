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
#include <Bentley/BeVersion.h>
#include <memory>

// WIP until licensing client is a proper dependency
// #include <Licensing/Client.h>
namespace Licensing
    {
    struct Client
        {
        static std::shared_ptr<Client> CreateFree(Utf8StringCR accessToken, BeVersionCR appVersion, Utf8StringCR projectId, BeFileNameCR cacheDbPath) { return std::make_shared<Client>(); }
        void StartApplication() {}
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
            static BeFileName* s_cacheDbPath;

            Licensing::ClientPtr m_client;
            Utf8String m_accessToken;
            BeVersion m_appVersion;
            Utf8String m_projectId;

        public:
            UlasClient(Utf8StringCR accessToken, Utf8StringCR appVersion, Utf8StringCR projectId) : m_accessToken(accessToken), m_projectId(projectId), m_appVersion(appVersion.c_str()) {}
            ~UlasClient();
            BentleyStatus Initialize();
            void StartTracking();
            void StopTracking();
        };
    };
