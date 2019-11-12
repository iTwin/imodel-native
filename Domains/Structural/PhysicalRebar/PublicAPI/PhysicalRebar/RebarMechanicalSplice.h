/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "PhysicalRebar.h"

BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct RebarMechanicalSplice : RebarEndDevice
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarMechanicalSplice, RebarEndDevice);
    friend struct RebarMechanicalSpliceHandler;

protected:
    explicit RebarMechanicalSplice(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarMechanicalSplice)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarMechanicalSplice)

    PHYSICALREBAR_EXPORT static RebarMechanicalSplicePtr Create(/*TODO: args*/);


}; // RebarMechanicalSplice

//=======================================================================================
//! 
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct RebarMechanicalSpliceType : RebarEndDeviceType
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarMechanicalSpliceType, RebarEndDeviceType);
    friend struct RebarMechanicalSpliceTypeHandler;

protected:
    explicit RebarMechanicalSpliceType(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarMechanicalSpliceType)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarMechanicalSpliceType)

    PHYSICALREBAR_EXPORT static RebarMechanicalSpliceTypePtr Create(/*TODO: args*/);


}; // RebarMechanicalSpliceType

END_BENTLEY_PHYSICALREBAR_NAMESPACE
