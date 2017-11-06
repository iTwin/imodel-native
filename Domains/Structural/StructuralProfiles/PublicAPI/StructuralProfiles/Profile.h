#pragma once

//__PUBLISH_SECTION_START__
#include <StructuralDomain/StructuralProfiles/StructuralProfilesDefinitions.h>

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! Profile
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Profile : Dgn::DefinitionElement
    {
    DGNELEMENT_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_Profile, Dgn::DefinitionElement);

    friend struct ProfileHandler;

protected:
    explicit Profile(CreateParams const& params) : T_Super(params), m_iCustomCardinalPointsIndex(0)
        {}

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(Profile)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(Profile)

    STRUCTURAL_DOMAIN_EXPORT void AddCustomCardinalPoint(Utf8CP name, DPoint2dCR coordinates);

private:
    BE_PROP_NAME(CustomCardinalPoints)
    BE_ECCLASS_NAME(CustomCardinalPointStruct)
    uint32_t m_iCustomCardinalPointsIndex;
    };

//=======================================================================================
//! The ElementHandler for ProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ProfileHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_Profile, Profile, ProfileHandler, Dgn::dgn_ElementHandler::Definition, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
