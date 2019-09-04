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
struct IReinforcable
{
protected:
    virtual Dgn::DgnElementCR _IReinforcableToDgnElement() const = 0;


}; // IReinforcable

END_BENTLEY_PHYSICALREBAR_NAMESPACE
