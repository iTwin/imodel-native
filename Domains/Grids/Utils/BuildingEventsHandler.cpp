/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/BuildingEventsHandler.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PrivateApi/BuildingEventsHandler.h"
#include "PublicApi/BuildingEvents.h"
#include "DgnClientFx/DgnClientApp.h"
#include "PublicApi\BuildingMessages.h"

BEGIN_BUILDING_NAMESPACE

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

    return set;
    }

END_BUILDING_NAMESPACE