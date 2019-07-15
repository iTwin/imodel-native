/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/iModelHub/Events/VersionEvent.h>

USING_NAMESPACE_BENTLEY_IMODELHUB

//---------------------------------------------------------------------------------------
//@bsimethod                                 Viktorija.Adomauskaite               06/2016
//---------------------------------------------------------------------------------------
VersionEvent::VersionEvent
(
Utf8String eventTopic,
Utf8String fromSubscriptionId,
Utf8String versionId,
Utf8String versionName,
Utf8String changeSetId,
Event::EventType versionEventType
)
    {
    m_eventTopic = eventTopic;
    m_fromEventSubscriptionId = fromSubscriptionId;
    m_versionId = versionId;
    m_versionName = versionName;
    m_changeSetId = changeSetId;
    m_versionEventType = versionEventType;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Viktorija.Adomauskaite               06/2016
//---------------------------------------------------------------------------------------
RefCountedPtr<struct VersionEvent> VersionEvent::Create
(
Utf8String eventTopic, 
Utf8String fromSubscriptionId, 
Utf8String versionId, 
Utf8String versionName, 
Utf8String changeSetId,
Event::EventType versionEventType
)
    {
    return new VersionEvent
                (
                eventTopic, 
                fromSubscriptionId, 
                versionId, 
                versionName, 
                changeSetId,
                versionEventType
                );
    }
