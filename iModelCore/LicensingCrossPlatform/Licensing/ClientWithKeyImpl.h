/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ClientWithKeyImpl.h $
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
typedef std::shared_ptr<struct ClientWithKeyImpl> ClientWithKeyImplPtr;

struct ClientWithKeyImpl : ClientImpl
    {
protected:
    Utf8String m_accessKey;
public:
    LICENSING_EXPORT ClientWithKeyImpl
        (
        Utf8StringCR accessKey,
        ClientInfoPtr clientInfo,
        BeFileNameCR db_path,
        bool offlineMode,
        IBuddiProviderPtr buddiProvider,
        IUlasProviderPtr ulasProvider,
        Utf8StringCR projectId,
        Utf8StringCR featureString,
        IHttpHandlerPtr httpHandler
        );
    LICENSING_EXPORT LicenseStatus StartApplication();
    LICENSING_EXPORT BentleyStatus StopApplication();

    LICENSING_EXPORT folly::Future<folly::Unit> SendUsageRealtimeWithKey();
    };

END_BENTLEY_LICENSING_NAMESPACE
