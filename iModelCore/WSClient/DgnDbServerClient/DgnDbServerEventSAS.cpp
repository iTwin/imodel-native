/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerEventSAS.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerEventSAS.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSAS::DgnDbServerEventSAS
(
Utf8String sasToken, 
Utf8String baseAddress
)
    {
    m_sasToken = sasToken;
    m_baseAddress = baseAddress;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
DgnDbServerEventSASPtr DgnDbServerEventSAS::Create
(
Utf8String sasToken,
Utf8String baseAddress
)
    {
    return DgnDbServerEventSASPtr(new DgnDbServerEventSAS(sasToken, baseAddress));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerEventSAS::GetSASToken()
    {
    return m_sasToken;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerEventSAS::GetBaseAddress()
    {
    return m_baseAddress;
    }