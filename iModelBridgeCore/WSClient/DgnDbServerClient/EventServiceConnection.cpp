/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/EventServiceConnection.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/EventServiceConnection.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
EventServiceConnection::EventServiceConnection(Utf8String sasToken, Utf8String nameSpace)
    {
    m_sasToken = sasToken;
    m_nameSpace = nameSpace;
    m_connectionId = nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
EventServiceConnection::EventServiceConnection(Utf8String sasToken, Utf8String nameSpace, Utf8String connectionId) : EventServiceConnection(sasToken, nameSpace)
    {
    m_connectionId = connectionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
EventServiceConnectionPtr EventServiceConnection::Create(Utf8String sasToken, Utf8String nameSpace)
    {
    return EventServiceConnectionPtr(new EventServiceConnection(sasToken, nameSpace));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
EventServiceConnectionPtr EventServiceConnection::Create(Utf8String sasToken, Utf8String nameSpace, Utf8String connectionId)
    {
    return EventServiceConnectionPtr(new EventServiceConnection(sasToken, nameSpace, connectionId));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
EventServiceConnectionPtr EventServiceConnection::CreateDefaultInfo()
    {
    return EventServiceConnectionPtr(new EventServiceConnection
                               (
                               "SharedAccessSignature sig=TOk40ce29TwpOYCFG7EWqHL5%2bmi9fIDX%2fYA0Ckv7Urs%3d&se=1463758026&skn=EventReceivePolicy&sr=https%3a%2f%2ftesthubjeehwan-ns.servicebus.windows.net%2ftest", 
                               "testhubjeehwan-ns"
                               ));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
Utf8String EventServiceConnection::GetSasToken()
    {
    return m_sasToken;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
Utf8String EventServiceConnection::GetNamespace()
    {
    return m_nameSpace;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
Utf8String EventServiceConnection::GetConnectionId()
    {
    return m_connectionId;
    }