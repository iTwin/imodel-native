/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>

// Includes for UniqueAspect. These includes are needed because UniqueAspect definition file does not include everything by itself
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ViewContext.h>
#include <DgnPlatform/TileTree.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct QuantityTakeoffAspect : Dgn::DgnElement::UniqueAspect
    {
    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT QuantityTakeoffAspect();
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
