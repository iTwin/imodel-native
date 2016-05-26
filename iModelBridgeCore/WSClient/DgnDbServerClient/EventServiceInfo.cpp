/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/EventServiceInfo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/EventServiceInfo.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
EventServiceInfo::EventServiceInfo(Utf8String sasToken, Utf8String nameSpace)
    {
    m_sasToken = sasToken;
    m_nameSpace = nameSpace;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
EventServiceInfoPtr EventServiceInfo::Create(Utf8String sasToken, Utf8String nameSpace)
    {
    return EventServiceInfoPtr(new EventServiceInfo(sasToken, nameSpace));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
EventServiceInfoPtr EventServiceInfo::CreateDefaultInfo()
    {
    return EventServiceInfoPtr(new EventServiceInfo
                               (
                               "SharedAccessSignature sig=TOk40ce29TwpOYCFG7EWqHL5%2bmi9fIDX%2fYA0Ckv7Urs%3d&se=1463758026&skn=EventReceivePolicy&sr=https%3a%2f%2ftesthubjeehwan-ns.servicebus.windows.net%2ftest", 
                               "testhubjeehwan-ns"
                               ));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
Utf8String EventServiceInfo::GetSasToken()
    {
    return m_sasToken;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
Utf8String EventServiceInfo::GetNamespace()
    {
    return m_nameSpace;
    }

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