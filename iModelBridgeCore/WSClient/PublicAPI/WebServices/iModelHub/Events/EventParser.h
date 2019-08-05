/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Events/Event.h>
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EventParser 
{
public:
    IMODELHUBCLIENT_EXPORT static EventPtr ParseEvent(Utf8CP responseContentType, Utf8String responseString);
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct LockEvent> GetLockEvent(EventPtr eventPtr);
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct ChangeSetPostPushEvent> GetChangeSetPostPushEvent(EventPtr eventPtr);
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct ChangeSetPrePushEvent> GetChangeSetPrePushEvent(EventPtr eventPtr);
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct CodeEvent> GetCodeEvent(EventPtr eventPtr);
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct DeletedEvent> GetDeletedEvent(EventPtr eventPtr);
    IMODELHUBCLIENT_EXPORT static RefCountedPtr<struct VersionEvent> GetVersionEvent(EventPtr eventPtr);
};

END_BENTLEY_IMODELHUB_NAMESPACE
