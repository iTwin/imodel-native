/*--------------------------------------------------------------------------------------+
|
|     $Source: PhysicalRebar/PublicAPI/PhysicalRebar/RebarTerminator.h $
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
struct RebarTerminator : RebarEndDevice
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarTerminator, RebarEndDevice);
    friend struct RebarTerminatorHandler;

protected:
    explicit RebarTerminator(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarTerminator)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarTerminator)

    PHYSICALREBAR_EXPORT static RebarTerminatorPtr Create(/*TODO: args*/);


}; // RebarTerminator

//=======================================================================================
//! 
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct RebarTerminatorType : RebarEndDeviceType
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarTerminatorType, RebarEndDeviceType);
    friend struct RebarTerminatorTypeHandler;

protected:
    explicit RebarTerminatorType(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarTerminatorType)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarTerminatorType)

    PHYSICALREBAR_EXPORT static RebarTerminatorTypePtr Create(/*TODO: args*/);


}; // RebarTerminatorType

END_BENTLEY_PHYSICALREBAR_NAMESPACE
