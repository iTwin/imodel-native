/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnViewUtils/BuildingEvents.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/BuildingDgnViewUtilsApi.h"
#include "PrivateApi/BuildingEventsHandler.h"

#include <DgnClientFx/DgnClientApp.h>
#include <algorithm>
//#include "PublicApi\BuildingMessages.h"



#define BUILDING_SPACEPLANNING_CLASS_Building                                 "Building"
#define BUILDING_SPACEPLANNING_CLASS_Site                                     "Site"

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BUILDING_SHARED_NAMESPACE

bvector<IToolSettingsChangedHandler*> BuildingEvents::s_RegisteredToolSettingsChangedHandlersVector;
bvector<ICreatePressedHandler*> BuildingEvents::s_RegisteredCreatePressedHandlersVector;
bvector<IResetPressedHandler*> BuildingEvents::s_RegisteredResetPressedHandlersVector;
bvector<IFrontStageValuesChangedHandler*> BuildingEvents::s_RegisteredFrontStageValuesChangedHandlersVector;
bvector<IFrontStageValuesRequestedHandler*> BuildingEvents::s_RegisteredFrontStageValuesRequestedHandlersVector;
typedef bvector<IElementChangedHandler*> ElementChangedHandlerVector;
typedef bmap<Dgn::DgnClassId, ElementChangedHandlerVector> ElementChangedHandlersMap;
ElementChangedHandlersMap BuildingEvents::s_RegisteredElementChangedHandlersMap;

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
* @bsimethod                                    Haroldas.Vitunskas                 08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::OnFrontStageValuesChange(JsonValueCR jsonValue)
    {
    DgnClientFx::DgnClientApp::App().Messages().Send(DgnClientFx::JsonMessage(BCS_MESSAGES_FRONTSTAGE_VALUES_CHANGED, jsonValue));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::OnFrontStageValuesRequest(JsonValueCR jsonValue)
    {
    DgnClientFx::DgnClientApp::App().Messages().Send(DgnClientFx::JsonMessage(BCS_MESSAGES_FRONTSTAGE_VALUES_REQUESTED, jsonValue));
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
* @bsimethod                                    Haroldas.Vitunskas                 08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterFrontStageValuesChangedHandler(IFrontStageValuesChangedHandler * iFrontStageValuesChangedHandler)
    {
    s_RegisteredFrontStageValuesChangedHandlersVector.push_back(iFrontStageValuesChangedHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterFrontStageValuesRequestedHandler(IFrontStageValuesRequestedHandler * iFrontStageValuesRequestedHandler)
    {
    s_RegisteredFrontStageValuesRequestedHandlersVector.push_back(iFrontStageValuesRequestedHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterElementChangedHandler(IElementChangedHandler* handler, Dgn::DgnClassId classId)
    {
    auto classEventHandlers = s_RegisteredElementChangedHandlersMap.find(classId);
    
    if (classEventHandlers == s_RegisteredElementChangedHandlersMap.end())
        {
        s_RegisteredElementChangedHandlersMap[classId] = ElementChangedHandlerVector{ handler };
        }
    else
        {
        auto existingHandler = std::find(s_RegisteredElementChangedHandlersMap[classId].begin(), s_RegisteredElementChangedHandlersMap[classId].end(), handler);
        if (s_RegisteredElementChangedHandlersMap[classId].end() == existingHandler)
            s_RegisteredElementChangedHandlersMap[classId].push_back(handler);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterSiteUpdatedHandler(ISiteUpdatedHandler* handler)
    {
    //RegisterElementChangedHandler(handler, BUILDING_SPACEPLANNING_CLASS_Building);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::RegisterBuildingUpdatedHandler(IBuildingUpdatedHandler* handler)
    {
    //RegisterElementChangedHandler(handler, BUILDING_SPACEPLANNING_CLASS_Site);
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
* @bsimethod                                    Haroldas.Vitunskas                 08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterFrontStageValuesChangedHandler(IFrontStageValuesChangedHandler * iFrontStageValuesChangedHandler)
    {
    auto it = std::find(s_RegisteredFrontStageValuesChangedHandlersVector.begin(), s_RegisteredFrontStageValuesChangedHandlersVector.end(), iFrontStageValuesChangedHandler);
    if (it != s_RegisteredFrontStageValuesChangedHandlersVector.end())
        s_RegisteredFrontStageValuesChangedHandlersVector.erase(it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterFrontStageValuesRequestedHandler(IFrontStageValuesRequestedHandler * iFrontStageValuesRequestedHandler)
    {
    auto it = std::find(s_RegisteredFrontStageValuesRequestedHandlersVector.begin(), s_RegisteredFrontStageValuesRequestedHandlersVector.end(), iFrontStageValuesRequestedHandler);
    if (it != s_RegisteredFrontStageValuesRequestedHandlersVector.end())
        s_RegisteredFrontStageValuesRequestedHandlersVector.erase(it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterElementChangedHandler(IElementChangedHandler* handler, Dgn::DgnClassId classId)
    {
    auto classEventHandlers = s_RegisteredElementChangedHandlersMap.find(classId);
    if (s_RegisteredElementChangedHandlersMap.end() == classEventHandlers)
        return;

    auto existingHandler = std::find(s_RegisteredElementChangedHandlersMap[classId].begin(), s_RegisteredElementChangedHandlersMap[classId].end(), handler);
    if (s_RegisteredElementChangedHandlersMap[classId].end() == existingHandler)
        return;

    s_RegisteredElementChangedHandlersMap[classId].erase(existingHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterBuildingUpdatedHandler(IBuildingUpdatedHandler* handler)
    {
    //UnregisterElementChangedHandler(handler, BUILDING_SPACEPLANNING_CLASS_Building);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::UnregisterSiteUpdatedHandler(ISiteUpdatedHandler* handler)
    {
    //UnregisterElementChangedHandler(handler, BUILDING_SPACEPLANNING_CLASS_Site);
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
    for (ICreatePressedHandler* handler : s_RegisteredCreatePressedHandlersVector)
        (*handler).OnCreatePressed(jsonValue, response);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyResetPressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response)
    {
    for (IResetPressedHandler* handler : s_RegisteredResetPressedHandlersVector)
        handler->OnResetPressed(jsonValue, response);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyFrontStageValuesChanged(JsonValueCR jsonValue, DgnClientFx::MessageResponse & response)
    {
    for (IFrontStageValuesChangedHandler * handler : s_RegisteredFrontStageValuesChangedHandlersVector)
        handler->OnFrontStageValuesChanged(jsonValue, response);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyFrontStageValuesRequested(JsonValueCR jsonValue, DgnClientFx::MessageResponse & response)
    {
    for (IFrontStageValuesRequestedHandler * handler : s_RegisteredFrontStageValuesRequestedHandlersVector)
        handler->OnFrontStageValuesRequested(jsonValue, response);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnClassId getClassId(DgnDbR db, DgnElementId id)
    {
    DgnElementCPtr elem = db.Elements().GetElement(id);
    BeAssert(elem.IsValid());
    return elem->GetElementClassId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
ElementChangedHandlerVector findHandlersInElementChangedMultimap(ElementChangedHandlersMap handlerMap, DgnDbR db, DgnElementId id)
    {
    DgnClassId classId = getClassId(db, id);
    return handlerMap[classId];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyElementInserted(Dgn::DgnDbR db, Dgn::DgnElementId elementId)
    {
    ElementChangedHandlerVector handlers = findHandlersInElementChangedMultimap(s_RegisteredElementChangedHandlersMap, db, elementId);

    for (auto handlerIt = handlers.begin(); handlerIt != handlers.end(); ++handlerIt)
        {
        (*handlerIt)->OnElementInserted(db, elementId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyElementUpdated(Dgn::DgnDbR db, Dgn::DgnElementId elementId)
    {
    ElementChangedHandlerVector handlers = findHandlersInElementChangedMultimap(s_RegisteredElementChangedHandlersMap, db, elementId);

    for (auto handlerIt = handlers.begin(); handlerIt != handlers.end(); ++handlerIt)
        {
        (*handlerIt)->OnElementUpdated(db, elementId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::NotifyElementDeleted(Dgn::DgnDbR db, Dgn::DgnElementId elementId)
    {
    ElementChangedHandlerVector handlers = findHandlersInElementChangedMultimap(s_RegisteredElementChangedHandlersMap, db, elementId);

    for (auto handlerIt = handlers.begin(); handlerIt != handlers.end(); ++handlerIt)
        {
        (*handlerIt)->OnElementDeleted(db, elementId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas                 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildingEvents::Initialize()
    {
    DgnClientFx::DgnClientApp::App().Messages().AddHandler(*BuildingEventsHandler::GetInstance());
    }

END_BUILDING_SHARED_NAMESPACE

