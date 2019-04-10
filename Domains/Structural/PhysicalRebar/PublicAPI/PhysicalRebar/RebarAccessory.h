/*--------------------------------------------------------------------------------------+
|
|     $Source: PhysicalRebar/PublicAPI/PhysicalRebar/RebarAccessory.h $
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
struct RebarAccessory : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarAccessory, Dgn::PhysicalElement);
    friend struct RebarAccessoryHandler;

protected:
    explicit RebarAccessory(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarAccessory)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarAccessory)

    PHYSICALREBAR_EXPORT static RebarAccessoryPtr Create(/*TODO: args*/);


}; // RebarAccessory

//=======================================================================================
//! 
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct RebarAccessoryType : Dgn::PhysicalType
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarAccessoryType, Dgn::PhysicalType);
    friend struct RebarAccessoryTypeHandler;

protected:
    explicit RebarAccessoryType(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarAccessoryType)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarAccessoryType)

    PHYSICALREBAR_EXPORT static RebarAccessoryTypePtr Create(/*TODO: args*/);


}; // RebarAccessoryType

END_BENTLEY_PHYSICALREBAR_NAMESPACE
