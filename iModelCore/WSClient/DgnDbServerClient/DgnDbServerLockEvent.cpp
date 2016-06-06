/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerLockEvent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerLockEvent.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             65/2016
//---------------------------------------------------------------------------------------
DgnDbServerLockEvent::DgnDbServerLockEvent(Utf8String repoId, Utf8String userId, Utf8String lockId, Utf8String lockType, Utf8String date)
    {
    m_repoId = repoId;
    m_userId = userId;
    m_lockId = lockId;
    m_lockType = lockType;
    m_date = date;
    m_eventType = "DgnDbServerLockEvent";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<struct DgnDbServerLockEvent> DgnDbServerLockEvent::Create(Utf8String repoId, Utf8String userId, Utf8String lockId, Utf8String lockType, Utf8String date)
    {
    return std::shared_ptr<struct DgnDbServerLockEvent>(new DgnDbServerLockEvent(repoId, userId, lockId, lockType, date));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetRepoId()
    {
    return m_repoId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetUserId()
    {
    return m_userId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetDate()
    {
    return m_date;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetLockId()
    {
    return m_lockId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetLockType()
    {
    return m_lockType;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerLockEvent::GetEventType()
    {
    return m_eventType;
    }