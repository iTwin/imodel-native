/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/EventServiceInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct EventServiceInfo> EventServiceInfoPtr;
typedef std::shared_ptr<struct EventServiceReceive> EventServiceReceivePtr;
//typedef AsyncResult<EventServiceReceive, WSError> EventServiceReceiveAysncResult;

struct EventServiceInfo
    {
    //__PUBLISH_SECTION_END__
    private:
        Utf8String m_sasToken;
        Utf8String m_nameSpace;

        EventServiceInfo(Utf8String sasToken, Utf8String nameSpace);
        //__PUBLISH_SECTION_START__
    public:
        DGNDBSERVERCLIENT_EXPORT static EventServiceInfoPtr Create(Utf8String sasToken, Utf8String nameSpace);
        DGNDBSERVERCLIENT_EXPORT static EventServiceInfoPtr CreateDefaultInfo();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetSasToken();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetNamespace();
    };

struct EventServiceReceive
    {
    private:
        bool m_isSuccess;
        Utf8String m_receivedMsg;

        EventServiceReceive(bool isSuccess, Utf8String receivedMsg);
    public:
        DGNDBSERVERCLIENT_EXPORT static EventServiceReceivePtr Create(bool isSuccess, Utf8String receivedMsg);
        DGNDBSERVERCLIENT_EXPORT bool IsSuccess();
        DGNDBSERVERCLIENT_EXPORT Utf8String ReceivedMessage();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
