/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEventConnection.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerEventConnection.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventConnection::DgnDbServerEventConnection(Utf8String sasToken, Utf8String nameSpace)
    {
    m_sasToken = sasToken;
    m_nameSpace = nameSpace;
    m_subscriptionId = nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventConnection::DgnDbServerEventConnection(Utf8String sasToken, Utf8String nameSpace, Utf8String subscriptionId) : DgnDbServerEventConnection(sasToken, nameSpace)
    {
    m_subscriptionId = subscriptionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventConnectionPtr DgnDbServerEventConnection::Create(Utf8String sasToken, Utf8String nameSpace)
    {
    return DgnDbServerEventConnectionPtr(new DgnDbServerEventConnection(sasToken, nameSpace));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventConnectionPtr DgnDbServerEventConnection::Create(Utf8String sasToken, Utf8String nameSpace, Utf8String subscriptionId)
    {
    return DgnDbServerEventConnectionPtr(new DgnDbServerEventConnection(sasToken, nameSpace, subscriptionId));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventConnectionPtr DgnDbServerEventConnection::CreateDefaultInfo()
    {
    return DgnDbServerEventConnectionPtr(new DgnDbServerEventConnection
                               (
                               "SharedAccessSignature sig=TOk40ce29TwpOYCFG7EWqHL5%2bmi9fIDX%2fYA0Ckv7Urs%3d&se=1463758026&skn=EventReceivePolicy&sr=https%3a%2f%2ftesthubjeehwan-ns.servicebus.windows.net%2ftest", 
                               "testhubjeehwan-ns"
                               ));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerEventConnection::GetSasToken()
    {
    return m_sasToken;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerEventConnection::GetNamespace()
    {
    return m_nameSpace;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             05/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerEventConnection::GetSubscriptionId()
    {
    return m_subscriptionId;
    }