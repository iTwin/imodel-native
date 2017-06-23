/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/BuildingEvents.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/BuildingEvents.h"
#include <DgnClientFx/DgnClientApp.h>
#include <algorithm>
#include "PublicApi\BuildingMessages.h"


USING_NAMESPACE_BENTLEY_DGN

BEGIN_BUILDING_NAMESPACE

bvector<IToolSettingsChangedHandler*> BuildingEvents::s_RegisteredToolSettingsChangedHandlersVector;
bvector<ICreatePressedHandler*> BuildingEvents::s_RegisteredCreatePressedHandlersVector;
bvector<IResetPressedHandler*> BuildingEvents::s_RegisteredResetPressedHandlersVector;
bmap<Utf8CP, IElementChangedHandler*> BuildingEvents::s_RegisteredElementChangedHandlersMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Wouter.Rombouts                 09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            BuildingEvents::OnToolSettingsValuesChange
(
JsonValueCR jsonValue
)
    {
    DgnClientFx::DgnClientApp::App ().Messages ().Send (DgnClientFx::JsonMessage (BUILDING_MESSAGES_VALUES_CHANGED, jsonValue));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::OnCreatePressed(JsonValueCR jsonValue)
    {
    DgnClientFx::DgnClientApp::App().Messages().Send(DgnClientFx::JsonMessage(BUILDING_MESSAGES_CREATE_PRESSED, jsonValue));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::OnResetPressed(JsonValueCR jsonValue)
    {
    DgnClientFx::DgnClientApp::App().Messages().Send(DgnClientFx::JsonMessage(BUILDING_MESSAGES_RESET_PRESSED, jsonValue));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterToolSettingChangedHandler(IToolSettingsChangedHandler* iToolSettingChangedHandler)
    {
    s_RegisteredToolSettingsChangedHandlersVector.push_back(iToolSettingChangedHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterCreatePressedHandler(ICreatePressedHandler* iCreatePressedHandler)
    {
    s_RegisteredCreatePressedHandlersVector.push_back(iCreatePressedHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterResetPressedHandler(IResetPressedHandler* iResetPressedHandler)
    {
    s_RegisteredResetPressedHandlersVector.push_back(iResetPressedHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterElementChangedHandler(IElementChangedHandler* handler, Utf8CP className)
    {
    s_RegisteredElementChangedHandlersMap[className] = handler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterSiteUpdatedHandler(ISiteUpdatedHandler* handler)
    {
    RegisterElementChangedHandler(handler, BUILDING_CLASS_BuildingEnvelope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterBuildingUpdatedHandler(IBuildingUpdatedHandler* handler)
    {
    RegisterElementChangedHandler(handler, BUILDING_CLASS_Site);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterToolSettingChangedHandler(IToolSettingsChangedHandler* iToolSettingChangedHandler)
    {
    bvector<IToolSettingsChangedHandler*>::iterator it = std::find(s_RegisteredToolSettingsChangedHandlersVector.begin(), s_RegisteredToolSettingsChangedHandlersVector.end(), iToolSettingChangedHandler);
    if (it != s_RegisteredToolSettingsChangedHandlersVector.end())
        s_RegisteredToolSettingsChangedHandlersVector.erase(it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterCreatePressedHandler(ICreatePressedHandler* iCreatePressedHandler)
    {
    bvector<ICreatePressedHandler*>::iterator it = std::find(s_RegisteredCreatePressedHandlersVector.begin(), s_RegisteredCreatePressedHandlersVector.end(), iCreatePressedHandler);
    if (it != s_RegisteredCreatePressedHandlersVector.end())
        s_RegisteredCreatePressedHandlersVector.erase(it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterResetPressedHandler(IResetPressedHandler* iResetPressedHandler)
    {
    bvector<IResetPressedHandler*>::iterator it = std::find(s_RegisteredResetPressedHandlersVector.begin(), s_RegisteredResetPressedHandlersVector.end(), iResetPressedHandler);
    if (it != s_RegisteredResetPressedHandlersVector.end())
        s_RegisteredResetPressedHandlersVector.erase(it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterElementChangedHandler(IElementChangedHandler* handler, Utf8CP className)
    {
    Utf8String classString = className;

    bmap<Utf8CP, IElementChangedHandler*>::iterator it = std::find_if(s_RegisteredElementChangedHandlersMap.begin(), s_RegisteredElementChangedHandlersMap.end(), [&](bpair<Utf8CP, IElementChangedHandler*> pair) {return 0 == classString.compare(pair.first); });
    if (it != s_RegisteredElementChangedHandlersMap.end())
        s_RegisteredElementChangedHandlersMap.erase(it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterBuildingUpdatedHandler(IBuildingUpdatedHandler* handler)
    {
    UnregisterElementChangedHandler(handler, BUILDING_CLASS_BuildingEnvelope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterSiteUpdatedHandler(ISiteUpdatedHandler* handler)
    {
    UnregisterElementChangedHandler(handler, BUILDING_CLASS_Site);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyToolSettingsChanged(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response)
    {
    for (bvector<IToolSettingsChangedHandler*>::iterator it = s_RegisteredToolSettingsChangedHandlersVector.begin(); it != s_RegisteredToolSettingsChangedHandlersVector.end(); ++it)
        {
        (*it)->OnToolSettingsChanged(jsonValue, response);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyCreatePressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response)
    {
    for each (ICreatePressedHandler* handler in s_RegisteredCreatePressedHandlersVector)
        (*handler).OnCreatePressed(jsonValue, response);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyResetPressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response)
    {
    for each (IResetPressedHandler* handler in s_RegisteredResetPressedHandlersVector)
        handler->OnResetPressed(jsonValue, response);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<Utf8CP, IElementChangedHandler*>::iterator findHandlerInElementChangedMap(bmap<Utf8CP, IElementChangedHandler*> handlerMap, DgnDbR db, DgnElementId id)
    {
    DgnElementCPtr elem = db.Elements().GetElement(id);
    if (!elem.IsValid())
        return handlerMap.end();

    Utf8String className = elem->GetElementClass()->GetName();
    if (0 == className.size())
        return handlerMap.end();

    return std::find_if(handlerMap.begin(), handlerMap.end(), [&className](bpair<Utf8CP, IElementChangedHandler*> pair) {return 0 == className.compare(pair.first); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyElementInserted(Dgn::DgnDbR db, Dgn::DgnElementId elementId)
    {
    bmap<Utf8CP, IElementChangedHandler*>::iterator it = findHandlerInElementChangedMap(s_RegisteredElementChangedHandlersMap, db, elementId);
    if (s_RegisteredElementChangedHandlersMap.end() != it)
        (*it).second->OnElementInserted(db, elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyElementUpdated(Dgn::DgnDbR db, Dgn::DgnElementId elementId)
    {
    bmap<Utf8CP, IElementChangedHandler*>::iterator it = findHandlerInElementChangedMap(s_RegisteredElementChangedHandlersMap, db, elementId);
    if (s_RegisteredElementChangedHandlersMap.end() != it)
        (*it).second->OnElementUpdated(db, elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyElementDeleted(Dgn::DgnDbR db, Dgn::DgnElementId elementId)
    {
    bmap<Utf8CP, IElementChangedHandler*>::iterator it = findHandlerInElementChangedMap(s_RegisteredElementChangedHandlersMap, db, elementId);
    if (s_RegisteredElementChangedHandlersMap.end() != it)
        (*it).second->OnElementDeleted(db, elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::Initialize()
    {
    DgnClientFx::DgnClientApp::App().Messages().AddHandler(*BuildingEventsHandler::GetInstance());
    }

END_BUILDING_NAMESPACE

