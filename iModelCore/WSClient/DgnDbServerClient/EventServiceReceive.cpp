/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/EventServiceReceive.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/EventServiceReceive.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
EventServiceReceive::EventServiceReceive(bool isSuccess, Utf8String receivedMsg)
    {
    m_isSuccess = isSuccess;
    m_receivedMsg = receivedMsg;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
EventServiceReceivePtr EventServiceReceive::Create(bool isSuccess, Utf8String receivedMsg)
    {
    return EventServiceReceivePtr(new EventServiceReceive(isSuccess, receivedMsg));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
bool EventServiceReceive::IsSuccess()
    {
    return m_isSuccess;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
Utf8String EventServiceReceive::ReceivedMessage()
    {
    return m_receivedMsg;
    }