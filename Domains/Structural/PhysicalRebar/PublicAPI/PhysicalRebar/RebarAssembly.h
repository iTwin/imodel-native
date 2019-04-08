/*--------------------------------------------------------------------------------------+
|
|     $Source: PhysicalRebar/PublicAPI/PhysicalRebar/RebarAssembly.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "PhysicalRebar.h"

BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE

//=======================================================================================
//! Grouping of related rebar sets that act together to reinforce a member.
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct RebarAssembly : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarAssembly, Dgn::PhysicalElement);
    friend struct RebarAssemblyHandler;

protected:
    explicit RebarAssembly(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarAssembly)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarAssembly)

    PHYSICALREBAR_EXPORT static RebarAssemblyPtr Create(/*TODO: args*/);


}; // RebarAssembly

END_BENTLEY_PHYSICALREBAR_NAMESPACE
