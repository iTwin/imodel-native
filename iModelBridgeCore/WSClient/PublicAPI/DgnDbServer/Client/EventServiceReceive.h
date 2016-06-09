/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/EventServiceReceive.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

typedef std::shared_ptr<struct EventServiceReceive> EventServiceReceivePtr;
//typedef AsyncResult<EventServiceReceive, WSError> EventServiceReceiveAysncResult;

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
