/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/GlobalEvents/GlobalEvent.h>
#include <WebServices/iModelHub/Common.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

typedef Utf8String GlobalEventSubscriptionId;
typedef RefCountedPtr<struct GlobalEventSubscription> GlobalEventSubscriptionPtr;
DEFINE_TASK_TYPEDEFS(GlobalEventSubscriptionPtr, GlobalEventSubscription);

//=======================================================================================
//@bsiclass                                      Karolis.Uzkuraitis             05/2018
//=======================================================================================
struct GlobalEventSubscription : RefCountedBase
    {
private:
    GlobalEventSubscriptionId m_subscriptionInstanceId;
    GlobalEventTypeSet m_eventTypes;

    GlobalEventSubscription(const GlobalEventSubscriptionId instanceId, const GlobalEventTypeSet eventTypes) : m_subscriptionInstanceId(instanceId), m_eventTypes(eventTypes) {}
public:
    static GlobalEventSubscriptionPtr Create(const Utf8String subscriptionId, const GlobalEventTypeSet eventTypes) { return new GlobalEventSubscription(subscriptionId, eventTypes); }
    GlobalEventSubscriptionId GetSubscriptionInstanceId() const { return m_subscriptionInstanceId; }
    GlobalEventTypeSet GetEventTypes() const { return m_eventTypes; }
    };

END_BENTLEY_IMODELHUB_NAMESPACE
