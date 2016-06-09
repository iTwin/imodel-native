/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/EventServiceConnection.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct EventServiceConnection> EventServiceConnectionPtr;

struct EventServiceConnection
    {
    //__PUBLISH_SECTION_END__
    private:
        Utf8String m_sasToken;
        Utf8String m_nameSpace;
        Utf8String m_subscriptionId;

        EventServiceConnection(Utf8String sasToken, Utf8String nameSpace);
        EventServiceConnection(Utf8String sasToken, Utf8String nameSpace, Utf8String subscriptionId);
        //__PUBLISH_SECTION_START__
    public:
        DGNDBSERVERCLIENT_EXPORT static EventServiceConnectionPtr Create(Utf8String sasToken, Utf8String nameSpace);
        DGNDBSERVERCLIENT_EXPORT static EventServiceConnectionPtr Create(Utf8String sasToken, Utf8String nameSpace, Utf8String subscriptionId);
        DGNDBSERVERCLIENT_EXPORT static EventServiceConnectionPtr CreateDefaultInfo();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetSasToken();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetNamespace();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetSubscriptionId();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
