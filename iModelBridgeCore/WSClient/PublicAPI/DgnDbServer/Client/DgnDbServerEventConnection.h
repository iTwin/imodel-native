/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerEventConnection.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct DgnDbServerEventConnection> DgnDbServerEventConnectionPtr;

struct DgnDbServerEventConnection
    {
    //__PUBLISH_SECTION_END__
    private:
        Utf8String m_sasToken;
        Utf8String m_nameSpace;
        Utf8String m_subscriptionId;

        DgnDbServerEventConnection(Utf8String sasToken, Utf8String nameSpace);
        DgnDbServerEventConnection(Utf8String sasToken, Utf8String nameSpace, Utf8String subscriptionId);
        //__PUBLISH_SECTION_START__
    public:
        DGNDBSERVERCLIENT_EXPORT static DgnDbServerEventConnectionPtr Create(Utf8String sasToken, Utf8String nameSpace);
        DGNDBSERVERCLIENT_EXPORT static DgnDbServerEventConnectionPtr Create(Utf8String sasToken, Utf8String nameSpace, Utf8String subscriptionId);
        DGNDBSERVERCLIENT_EXPORT static DgnDbServerEventConnectionPtr CreateDefaultInfo();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetSasToken();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetNamespace();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetSubscriptionId();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
