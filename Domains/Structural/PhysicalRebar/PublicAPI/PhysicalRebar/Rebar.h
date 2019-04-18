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
struct Rebar : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_Rebar, Dgn::PhysicalElement);
    friend struct RebarHandler;

protected:
    explicit Rebar(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(Rebar)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(Rebar)

    PHYSICALREBAR_EXPORT static RebarPtr Create(/*TODO: args*/);


}; // Rebar

//=======================================================================================
//! 
//! @ingroup GROUP_PhysicalRebar
//=======================================================================================
struct RebarType : Dgn::PhysicalType
{
    DGNELEMENT_DECLARE_MEMBERS(SPR_CLASS_RebarType, Dgn::PhysicalType);
    friend struct RebarTypeHandler;

protected:
    explicit RebarType(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(RebarType)
    DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(RebarType)

    PHYSICALREBAR_EXPORT static RebarTypePtr Create(/*TODO: args*/);

public:
    Bentley.Geometry.Common.IGeometry GetShape() const { return GetPropertyValue(SPR_PROP_RebarType_Shape); }
    PHYSICALREBAR_EXPORT void SetShape(Bentley.Geometry.Common.IGeometry val);
    Bentley.Geometry.Common.IGeometry GetSimplifiedShape() const { return GetPropertyValue(SPR_PROP_RebarType_SimplifiedShape); }
    PHYSICALREBAR_EXPORT void SetSimplifiedShape(Bentley.Geometry.Common.IGeometry val);

}; // RebarType

END_BENTLEY_PHYSICALREBAR_NAMESPACE
