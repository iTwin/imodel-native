/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnDbServerClient/DgnDbServerRevisionEvent.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbServer/Client/DgnDbServerRevisionEvent.h>
#include "DgnDbServerUtils.h"

USING_NAMESPACE_BENTLEY_DGNDBSERVER

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             65/2016
//---------------------------------------------------------------------------------------
DgnDbServerRevisionEvent::DgnDbServerRevisionEvent(Utf8String repoId, Utf8String userId, Utf8String revisionId, Utf8String date)
    {
    m_repoId = repoId;
    m_userId = userId;
    m_revisionId = revisionId;
    m_date = date;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
std::shared_ptr<struct DgnDbServerRevisionEvent> DgnDbServerRevisionEvent::Create(Utf8String repoId, Utf8String userId, Utf8String revisionId, Utf8String date)
    {
    return std::shared_ptr<struct DgnDbServerRevisionEvent>(new DgnDbServerRevisionEvent(repoId, userId, revisionId, date));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionEvent::GetRepoId()
    {
    return m_repoId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionEvent::GetUserId()
    {
    return m_userId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionEvent::GetDate()
    {
    return m_date;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String DgnDbServerRevisionEvent::GetRevisionId()
    {
    return m_revisionId;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
const type_info& DgnDbServerRevisionEvent::GetEventType()
    {
    const type_info& tp = typeid(this);
    return tp;
    }