/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbServerRevisionEvent.h $
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

struct DgnDbServerRevisionEvent : public IDgnDbServerEvent
    {
    private:
        Utf8String m_repoId;
        Utf8String m_userId;
        Utf8String m_revisionId;
        Utf8String m_revisionIndex;
        Utf8String m_date;
        DgnDbServerRevisionEvent
                                (
                                Utf8String repoId,
                                Utf8String userId, 
                                Utf8String revisionId, 
                                Utf8String revisionIndex, 
                                Utf8String date
                                );

    public:
        DGNDBSERVERCLIENT_EXPORT static std::shared_ptr<struct DgnDbServerRevisionEvent> Create
                                                                                               (
                                                                                               Utf8String repoId, 
                                                                                               Utf8String userId, 
                                                                                               Utf8String revisionId, 
                                                                                               Utf8String revisionIndex, 
                                                                                               Utf8String date
                                                                                               );
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetRepoId();
        DGNDBSERVERCLIENT_EXPORT virtual Utf8String GetUserId();
        DGNDBSERVERCLIENT_EXPORT virtual const type_info& GetEventType();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetDate();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetRevisionId();
        DGNDBSERVERCLIENT_EXPORT Utf8String GetRevisionIndex();
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE