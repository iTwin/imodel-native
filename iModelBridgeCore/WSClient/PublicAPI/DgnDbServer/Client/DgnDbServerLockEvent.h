/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerLockEvent.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/Client/DgnDbServerEvent.h>
#include <DgnDbServer/DgnDbServerCommon.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGNPLATFORM

//typedef std::shared_ptr<struct DgnDbServerLockEvent> DgnDbServerLockEventPtr;

struct DgnDbServerLockEvent : public DgnDbServerEvent::GenericEvent
    {
    private:
        Utf8String m_repoId;
        Utf8String m_userId;
        Utf8String m_date;
        Utf8String m_objectId;
        Utf8String m_lockType;
        Utf8String m_lockLevel;
        Utf8String m_briefcaseId;
        Utf8String m_releasedWithRevision;
        DgnDbServerLockEvent
                            (
                            Utf8String repoId, 
                            Utf8String userId, 
                            Utf8String objectId, 
                            Utf8String lockType, 
                            Utf8String lockLevel, 
                            Utf8String briefcaseId, 
                            Utf8String releasedWithRevision, 
                            Utf8String date
                            );

    public:
        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerLockEvent> Create
                                                                                           (
                                                                                           Utf8String repoId, 
                                                                                           Utf8String userId, 
                                                                                           Utf8String objectId, 
                                                                                           Utf8String lockType, 
                                                                                           Utf8String lockLevel, 
                                                                                           Utf8String briefcaseId, 
                                                                                           Utf8String releasedWithRevision, 
                                                                                           Utf8String date
                                                                                           );
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetRepoId();
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetUserId();
        DGNDBSERVERCLIENT_EXPORT virtual const type_info& GetEventType();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetDate();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetObjectId();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetLockType();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetLockLevel();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetBriefcaseId();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetReleasedWithRevision();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE