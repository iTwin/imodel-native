/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/BuildingEvents.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingMacros.h>
#include <DgnClientFx/Messages.h>
#include "Interfaces/IToolSettingsChangedHandler.h"
#include "Interfaces/ICreatePressedHandler.h"
#include "Interfaces/IResetPressedHandler.h"
#include "Interfaces\IBuildingUpdatedHandler.h"
#include "Interfaces\ISiteUpdatedHandler.h"
#include "Interfaces\IElementChangedHandler.h"
#include "../PrivateApi/BuildingEventsHandler.h"

BEGIN_BUILDING_NAMESPACE

// BuildingEvents is a static class for registering building event handlers and firing events
class BuildingEvents
    {
	private:
		static bvector<IToolSettingsChangedHandler*> s_RegisteredToolSettingsChangedHandlersVector;
        static bvector<ICreatePressedHandler*> s_RegisteredCreatePressedHandlersVector;
        static bvector<IResetPressedHandler*> s_RegisteredResetPressedHandlersVector;
        static bmap<Utf8CP, IElementChangedHandler*> s_RegisteredElementChangedHandlersMap;
        
        friend struct BuildingEventsHandler;

    private:
        BuildingEvents () {};
		BUILDINGUTILS_EXPORT static void NotifyToolSettingsChanged(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response);
        BUILDINGUTILS_EXPORT static void NotifyCreatePressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response);
        BUILDINGUTILS_EXPORT static void NotifyResetPressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response);
    public:
        BUILDINGUTILS_EXPORT static void NotifyElementUpdated   (Dgn::DgnDbR db, Dgn::DgnElementId elementId);
        BUILDINGUTILS_EXPORT static void NotifyElementInserted   (Dgn::DgnDbR db, Dgn::DgnElementId elementId);
        BUILDINGUTILS_EXPORT static void NotifyElementDeleted   (Dgn::DgnDbR db, Dgn::DgnElementId elementId);

        BUILDINGUTILS_EXPORT static void OnToolSettingsValuesChange (JsonValueCR jsonValue);
        BUILDINGUTILS_EXPORT static void OnCreatePressed(JsonValueCR jsonValue);
        BUILDINGUTILS_EXPORT static void OnResetPressed(JsonValueCR jsonValue);
	
        BUILDINGUTILS_EXPORT static void RegisterToolSettingChangedHandler(IToolSettingsChangedHandler* iToolSettingChangedHandler);
        BUILDINGUTILS_EXPORT static void RegisterCreatePressedHandler(ICreatePressedHandler* iCreatePressedHandler);
        BUILDINGUTILS_EXPORT static void RegisterResetPressedHandler(IResetPressedHandler* iCreatePressedHandler);

        //For now supports spaces, openings, sites and building envelopes
        BUILDINGUTILS_EXPORT static void RegisterElementChangedHandler  (IElementChangedHandler* handler, Utf8CP className);
        BUILDINGUTILS_EXPORT static void RegisterSiteUpdatedHandler  (ISiteUpdatedHandler* handler);
        BUILDINGUTILS_EXPORT static void RegisterBuildingUpdatedHandler  (IBuildingUpdatedHandler* handler);

        BUILDINGUTILS_EXPORT static void UnregisterToolSettingChangedHandler(IToolSettingsChangedHandler* iToolSettingChangedHandler);
        BUILDINGUTILS_EXPORT static void UnregisterCreatePressedHandler(ICreatePressedHandler* iCreatePressedHandler);
        BUILDINGUTILS_EXPORT static void UnregisterResetPressedHandler(IResetPressedHandler* iResetPressedHandler);
        
        BUILDINGUTILS_EXPORT static void UnregisterElementChangedHandler    (IElementChangedHandler* handler, Utf8CP className);
        BUILDINGUTILS_EXPORT static void UnregisterSiteUpdatedHandler(ISiteUpdatedHandler* handler);
        BUILDINGUTILS_EXPORT static void UnregisterBuildingUpdatedHandler(IBuildingUpdatedHandler* handler);

        BUILDINGUTILS_EXPORT static void Initialize();
    };

END_BUILDING_NAMESPACE