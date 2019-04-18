/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "DgnClientFx/DgnClientApp.h"
#include "PublicApi/BuildingDgnViewUtilsApi.h"

#include "PrivateApi/BuildingEventsHandler.h"
//#include "PublicApi/BuildingEvents.h"
//#include "PublicApi\BuildingMessages.h"

BEGIN_BUILDING_SHARED_NAMESPACE

BuildingEventsHandlerPtr BuildingEventsHandler::s_instance = NULL;

/*---------------------------------------------------------------------------------------
* @bsimethod                                    Haroldas.Vitunskas                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
BuildingEventsHandlerPtr BuildingEventsHandler::GetInstance()
    {
    if (s_instance == nullptr)
        s_instance = new BuildingEventsHandler();
    return s_instance;
    };
/*---------------------------------------------------------------------------------------
* @bsimethod                                    Haroldas.Vitunskas                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
BuildingEventsHandler::~BuildingEventsHandler()
    {
    
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                    Haroldas.Vitunskas                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEventsHandler::_HandleMessage(DgnClientFx::JsonMessage const& message, DgnClientFx::MessageResponse& response)
    {
    if (strcmp(message.GetType(), BUILDING_MESSAGES_VALUES_CHANGED) == 0)
        BuildingEvents::NotifyToolSettingsChanged(message.GetJson(), response);
    else if (strcmp(message.GetType(), BUILDING_MESSAGES_CREATE_PRESSED) == 0)
        BuildingEvents::NotifyCreatePressed(message.GetJson(), response);
    else if (strcmp(message.GetType(), BUILDING_MESSAGES_RESET_PRESSED) == 0)
        BuildingEvents::NotifyResetPressed(message.GetJson(), response);
    else if (strcmp(message.GetType(), BCS_MESSAGES_FRONTSTAGE_VALUES_CHANGED) == 0)
        BuildingEvents::NotifyFrontStageValuesChanged(message.GetJson(), response);
    else if (strcmp(message.GetType(), BCS_MESSAGES_FRONTSTAGE_VALUES_REQUESTED) == 0)
        BuildingEvents::NotifyFrontStageValuesRequested(message.GetJson(), response);
    return;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                    Haroldas.Vitunskas                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bset<Utf8String> BuildingEventsHandler::_GetMessageTypes() const
    {
    bset<Utf8String> set;

    set.insert(BUILDING_MESSAGES_VALUES_CHANGED);
    set.insert(BUILDING_MESSAGES_CREATE_PRESSED);
    set.insert(BUILDING_MESSAGES_RESET_PRESSED);
    set.insert(BCS_MESSAGES_FRONTSTAGE_VALUES_CHANGED);
    set.insert(BCS_MESSAGES_FRONTSTAGE_VALUES_REQUESTED);

    return set;
    }

END_BUILDING_SHARED_NAMESPACE