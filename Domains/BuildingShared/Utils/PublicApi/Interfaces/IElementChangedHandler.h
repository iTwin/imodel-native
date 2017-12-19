/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/Interfaces/IElementChangedHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingShared/BuildingSharedMacros.h>
#include <DgnClientFx/Messages.h>

BEGIN_BUILDING_SHARED_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE IElementChangedHandler
    {
    BUILDINGSHAREDUNITS_EXPORT virtual void OnElementInserted(Dgn::DgnDbR db, Dgn::DgnElementId elementId) = 0;
    BUILDINGSHAREDUNITS_EXPORT virtual void OnElementUpdated(Dgn::DgnDbR db, Dgn::DgnElementId elementId) = 0;
    BUILDINGSHAREDUNITS_EXPORT virtual void OnElementDeleted(Dgn::DgnDbR db, Dgn::DgnElementId elementId) = 0;
    };

END_BUILDING_SHARED_NAMESPACE