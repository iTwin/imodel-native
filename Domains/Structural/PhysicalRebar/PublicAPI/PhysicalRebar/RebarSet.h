/*--------------------------------------------------------------------------------------+
|
|     $Source: PhysicalRebar/PublicAPI/PhysicalRebar/RebarSet.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "PhysicalRebar.h"

BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE

//=======================================================================================
//! Grouping of similar rebars the reinforce a member or portion of a member.
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct RebarSet : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarSet, Dgn::PhysicalElement);
    friend struct RebarSetHandler;

protected:
    explicit RebarSet(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarSet)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarSet)

    PHYSICALREBAR_EXPORT static RebarSetPtr Create(/*TODO: args*/);


}; // RebarSet

END_BENTLEY_PHYSICALREBAR_NAMESPACE
