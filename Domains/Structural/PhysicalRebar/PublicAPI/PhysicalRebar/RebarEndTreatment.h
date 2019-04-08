/*--------------------------------------------------------------------------------------+
|
|     $Source: PhysicalRebar/PublicAPI/PhysicalRebar/RebarEndTreatment.h $
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
struct RebarEndTreatment : Dgn::DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarEndTreatment, Dgn::DefinitionElement);
    friend struct RebarEndTreatmentHandler;

protected:
    explicit RebarEndTreatment(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarEndTreatment)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarEndTreatment)

    PHYSICALREBAR_EXPORT static RebarEndTreatmentPtr Create(/*TODO: args*/);


}; // RebarEndTreatment

END_BENTLEY_PHYSICALREBAR_NAMESPACE
