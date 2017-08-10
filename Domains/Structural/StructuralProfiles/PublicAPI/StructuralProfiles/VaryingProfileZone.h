#pragma once
//__PUBLISH_SECTION_START__
#include "StructuralProfilesDefinitions.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE
//TODO: MutiAspect! currently this class  not in compile process
struct EXPORT_VTABLE_ATTRIBUTE VaryingProfileZone : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_VaryingProfileZone, Dgn::PhysicalElement);

    friend struct VaryingProfileZoneHandler;

protected:
    explicit VaryingProfileZone(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(VaryingProfileZone)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(VaryingProfileZone)

    STRUCTURAL_DOMAIN_EXPORT static VaryingProfileZonePtr Create(Dgn::PhysicalModelR model);
};

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE VaryingProfileZoneHandler : Dgn::dgn_ElementHandler::Physical
{
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_VaryingProfileZone, VaryingProfileZone, VaryingProfileZoneHandler, Dgn::dgn_ElementHandler::Physical, STRUCTURAL_DOMAIN_EXPORT)
};

END_BENTLEY_STRUCTURAL_NAMESPACE