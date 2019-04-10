/*--------------------------------------------------------------------------------------+
|
|     $Source: IModelJsNative.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
#else
namespace Licensing
{
typedef std::shared_ptr<struct SaasClient> SaasClientPtr;
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
        BentleyStatus TrackUsage(Utf8StringCR accessToken, Utf8StringCR appVersion, Utf8StringCR projectId) const;
        };
    };
