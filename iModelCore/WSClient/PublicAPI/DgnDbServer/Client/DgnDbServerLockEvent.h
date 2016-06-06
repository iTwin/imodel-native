/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerLockEvent.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/Client/IDgnDbServerEvent.h>
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

//typedef std::shared_ptr<struct DgnDbServerLockEvent> DgnDbServerLockEventPtr;

struct DgnDbServerLockEvent : public IDgnDbServerEvent
    {
    private:
        Utf8String m_repoId;
        Utf8String m_userId;
        Utf8String m_date;
        Utf8String m_lockId;
        Utf8String m_lockType;
        Utf8String m_eventType;
        DgnDbServerLockEvent(Utf8String repoId, Utf8String userId, Utf8String lockId, Utf8String lockType, Utf8String date);

    public:
        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerLockEvent> Create(Utf8String repoId, Utf8String userId, Utf8String lockId, Utf8String lockType, Utf8String date);
        DGNDBSERVERCLIENT_EXPORT Utf8String GetRepoId() override;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetUserId() override;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetEventType() override;
        DGNDBSERVERCLIENT_EXPORT Utf8String GetDate();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetLockId();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetLockType();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE