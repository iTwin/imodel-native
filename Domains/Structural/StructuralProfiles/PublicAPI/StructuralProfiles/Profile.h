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
    explicit Profile(CreateParams const& params);

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(Profile)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(Profile)

    STRUCTURAL_DOMAIN_EXPORT bool AddCustomCardinalPoint(Utf8CP name, DPoint2dCR coordinates);
    STRUCTURAL_DOMAIN_EXPORT bool AddCustomCardinalPoint(Utf8CP name, double x, double y);
    STRUCTURAL_DOMAIN_EXPORT bool RemoveAllCustomCardinalPoints();
    STRUCTURAL_DOMAIN_EXPORT bool RemoveCustomCardinalPoint(Utf8CP name);
    STRUCTURAL_DOMAIN_EXPORT bool SetCustomCardinalPoint(Utf8CP name, DPoint2dCR coordinates);
    STRUCTURAL_DOMAIN_EXPORT bool SetCustomCardinalPoint(Utf8CP name, double x, double y);

protected:
    ECN::StandaloneECEnablerPtr GetCustomCardinalPointsEnabler();
    STRUCTURAL_DOMAIN_EXPORT uint32_t CustomCardinalPointsCount();
    STRUCTURAL_DOMAIN_EXPORT bool FindCustomCardinalPointIndexByName(uint32_t& index, Utf8CP name);
    STRUCTURAL_DOMAIN_EXPORT bool LookupCustomCardinalPointByName(Utf8CP name);
private:
    BE_PROP_NAME(CustomCardinalPoints)
    BE_ECCLASS_NAME(CustomCardinalPointStruct)
    };

//=======================================================================================
//! The ElementHandler for ProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ProfileHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_Profile, Profile, ProfileHandler, Dgn::dgn_ElementHandler::Definition, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
