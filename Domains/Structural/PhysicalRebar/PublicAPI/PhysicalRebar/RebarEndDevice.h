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
struct RebarEndDevice : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarEndDevice, Dgn::PhysicalElement);
    friend struct RebarEndDeviceHandler;

protected:
    explicit RebarEndDevice(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarEndDevice)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarEndDevice)

    PHYSICALREBAR_EXPORT static RebarEndDevicePtr Create(/*TODO: args*/);


}; // RebarEndDevice

//=======================================================================================
//! 
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct RebarEndDeviceType : Dgn::PhysicalType
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarEndDeviceType, Dgn::PhysicalType);
    friend struct RebarEndDeviceTypeHandler;

protected:
    explicit RebarEndDeviceType(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarEndDeviceType)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarEndDeviceType)

    PHYSICALREBAR_EXPORT static RebarEndDeviceTypePtr Create(/*TODO: args*/);


}; // RebarEndDeviceType

END_BENTLEY_PHYSICALREBAR_NAMESPACE
