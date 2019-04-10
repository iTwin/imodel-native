/*--------------------------------------------------------------------------------------+
|
|     $Source: PhysicalRebar/PublicAPI/PhysicalRebar/RebarMaterial.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "PhysicalRebar.h"

BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct RebarMaterial
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarMaterial, );
    friend struct RebarMaterialHandler;

protected:
    explicit RebarMaterial(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarMaterial)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarMaterial)

    PHYSICALREBAR_EXPORT static RebarMaterialPtr Create(/*TODO: args*/);


}; // RebarMaterial

END_BENTLEY_PHYSICALREBAR_NAMESPACE
