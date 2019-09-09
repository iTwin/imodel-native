/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

struct IElementChangedHandler
    {
    virtual void OnElementInserted(Dgn::DgnDbR db, Dgn::DgnElementId elementId) = 0;
    virtual void OnElementUpdated(Dgn::DgnDbR db, Dgn::DgnElementId elementId) = 0;
    virtual void OnElementDeleted(Dgn::DgnDbR db, Dgn::DgnElementId elementId) = 0;
    };

END_BUILDING_SHARED_NAMESPACE