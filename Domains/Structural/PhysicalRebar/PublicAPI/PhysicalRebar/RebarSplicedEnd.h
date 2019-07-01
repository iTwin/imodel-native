/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "PhysicalRebar.h"

BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct RebarSplicedEnd
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarSplicedEnd, );
    friend struct RebarSplicedEndHandler;

protected:
    explicit RebarSplicedEnd(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarSplicedEnd)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarSplicedEnd)

    PHYSICALREBAR_EXPORT static RebarSplicedEndPtr Create(/*TODO: args*/);


}; // RebarSplicedEnd

END_BENTLEY_PHYSICALREBAR_NAMESPACE
