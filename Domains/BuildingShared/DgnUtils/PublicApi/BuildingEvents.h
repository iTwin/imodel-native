/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnUtils/PublicApi/BuildingEvents.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnClientFx/Messages.h>
#include "Interfaces/IToolSettingsChangedHandler.h"
#include "Interfaces/ICreatePressedHandler.h"
#include "Interfaces/IResetPressedHandler.h"
#include "Interfaces\IBuildingUpdatedHandler.h"
#include "Interfaces\ISiteUpdatedHandler.h"
#include "Interfaces\IElementChangedHandler.h"
#include "Interfaces\IFrontStageValuesChangedHandler.h"
#include "Interfaces\IFrontStageValuesRequestedHandler.h"
//#include "../PrivateApi/BuildingEventsHandler.h"

BEGIN_BUILDING_SHARED_NAMESPACE

// BuildingEvents is a static class for registering building event handlers and firing events
class BuildingEvents
    {
	private:
		static bvector<IToolSettingsChangedHandler*> s_RegisteredToolSettingsChangedHandlersVector;
        static bvector<ICreatePressedHandler*> s_RegisteredCreatePressedHandlersVector;
        static bvector<IResetPressedHandler*> s_RegisteredResetPressedHandlersVector;
        static bmap<Dgn::DgnClassId, bvector<IElementChangedHandler*>> s_RegisteredElementChangedHandlersMap;
        static bvector<IFrontStageValuesChangedHandler*> s_RegisteredFrontStageValuesChangedHandlersVector;
        static bvector<IFrontStageValuesRequestedHandler*> s_RegisteredFrontStageValuesRequestedHandlersVector;
        
        friend struct BuildingEventsHandler;

    private:
        BuildingEvents () {};
        BUILDINGSHAREDDGNUTILS_EXPORT static void NotifyToolSettingsChanged(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response);
        BUILDINGSHAREDDGNUTILS_EXPORT static void NotifyCreatePressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response);
        BUILDINGSHAREDDGNUTILS_EXPORT static void NotifyResetPressed(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response);
        BUILDINGSHAREDDGNUTILS_EXPORT static void NotifyFrontStageValuesChanged(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response);
        BUILDINGSHAREDDGNUTILS_EXPORT static void NotifyFrontStageValuesRequested(JsonValueCR jsonValue, DgnClientFx::MessageResponse& response);
    public:
        BUILDINGSHAREDDGNUTILS_EXPORT static void NotifyElementUpdated   (Dgn::DgnDbR db, Dgn::DgnElementId elementId);
        BUILDINGSHAREDDGNUTILS_EXPORT static void NotifyElementInserted   (Dgn::DgnDbR db, Dgn::DgnElementId elementId);
        BUILDINGSHAREDDGNUTILS_EXPORT static void NotifyElementDeleted   (Dgn::DgnDbR db, Dgn::DgnElementId elementId);

        BUILDINGSHAREDDGNUTILS_EXPORT static void OnToolSettingsValuesChange (JsonValueCR jsonValue);
        BUILDINGSHAREDDGNUTILS_EXPORT static void OnCreatePressed(JsonValueCR jsonValue);
        BUILDINGSHAREDDGNUTILS_EXPORT static void OnResetPressed(JsonValueCR jsonValue);
        BUILDINGSHAREDDGNUTILS_EXPORT static void OnFrontStageValuesChange(JsonValueCR jsonValue);
        BUILDINGSHAREDDGNUTILS_EXPORT static void OnFrontStageValuesRequest(JsonValueCR jsonValue);
	
        BUILDINGSHAREDDGNUTILS_EXPORT static void RegisterToolSettingChangedHandler(IToolSettingsChangedHandler* iToolSettingChangedHandler);
        BUILDINGSHAREDDGNUTILS_EXPORT static void RegisterCreatePressedHandler(ICreatePressedHandler* iCreatePressedHandler);
        BUILDINGSHAREDDGNUTILS_EXPORT static void RegisterResetPressedHandler(IResetPressedHandler* iCreatePressedHandler);
        BUILDINGSHAREDDGNUTILS_EXPORT static void RegisterFrontStageValuesChangedHandler(IFrontStageValuesChangedHandler* iFrontStageValuesChangedHandler);
        BUILDINGSHAREDDGNUTILS_EXPORT static void RegisterFrontStageValuesRequestedHandler(IFrontStageValuesRequestedHandler* iFrontStageValuesRequestedHandler);

        //For now supports spaces, openings, sites and building envelopes
        BUILDINGSHAREDDGNUTILS_EXPORT static void RegisterElementChangedHandler  (IElementChangedHandler* handler, Dgn::DgnClassId classId);
        BUILDINGSHAREDDGNUTILS_EXPORT static void RegisterSiteUpdatedHandler  (ISiteUpdatedHandler* handler);
        BUILDINGSHAREDDGNUTILS_EXPORT static void RegisterBuildingUpdatedHandler  (IBuildingUpdatedHandler* handler);

        BUILDINGSHAREDDGNUTILS_EXPORT static void UnregisterToolSettingChangedHandler(IToolSettingsChangedHandler* iToolSettingChangedHandler);
        BUILDINGSHAREDDGNUTILS_EXPORT static void UnregisterCreatePressedHandler(ICreatePressedHandler* iCreatePressedHandler);
        BUILDINGSHAREDDGNUTILS_EXPORT static void UnregisterResetPressedHandler(IResetPressedHandler* iResetPressedHandler);
        BUILDINGSHAREDDGNUTILS_EXPORT static void UnregisterFrontStageValuesChangedHandler(IFrontStageValuesChangedHandler* iFrontStageValuesChangedHandler);
        BUILDINGSHAREDDGNUTILS_EXPORT static void UnregisterFrontStageValuesRequestedHandler(IFrontStageValuesRequestedHandler* iFrontStageValuesRequestedHandler);
        
        BUILDINGSHAREDDGNUTILS_EXPORT static void UnregisterElementChangedHandler    (IElementChangedHandler* handler, Dgn::DgnClassId classId);
        BUILDINGSHAREDDGNUTILS_EXPORT static void UnregisterSiteUpdatedHandler(ISiteUpdatedHandler* handler);
        BUILDINGSHAREDDGNUTILS_EXPORT static void UnregisterBuildingUpdatedHandler(IBuildingUpdatedHandler* handler);

        BUILDINGSHAREDDGNUTILS_EXPORT static void Initialize();
    };

END_BUILDING_SHARED_NAMESPACE