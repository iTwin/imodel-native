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
struct RebarSize : Dgn::DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarSize, Dgn::DefinitionElement);
    friend struct RebarSizeHandler;

protected:
    explicit RebarSize(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarSize)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarSize)

    PHYSICALREBAR_EXPORT static RebarSizePtr Create(/*TODO: args*/);

public:
    string GetName() const { return GetPropertyValue(SPR_PROP_RebarSize_Name); }
    PHYSICALREBAR_EXPORT void SetName(string val);
    double GetDiameter() const { return GetPropertyValue(SPR_PROP_RebarSize_Diameter); }
    PHYSICALREBAR_EXPORT void SetDiameter(double val);
    double GetArea() const { return GetPropertyValue(SPR_PROP_RebarSize_Area); }
    PHYSICALREBAR_EXPORT void SetArea(double val);
    string GetPublisher() const { return GetPropertyValue(SPR_PROP_RebarSize_Publisher); }
    PHYSICALREBAR_EXPORT void SetPublisher(string val);

}; // RebarSize

END_BENTLEY_PHYSICALREBAR_NAMESPACE
