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

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(Profile)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(Profile)

    STRUCTURAL_DOMAIN_EXPORT bool AddCardinalPoint(Utf8CP name, DPoint2dCR coordinates);
    STRUCTURAL_DOMAIN_EXPORT bool AddCardinalPoint(Utf8CP name, double x, double y);
    STRUCTURAL_DOMAIN_EXPORT bool RemoveAllCardinalPoints();
    STRUCTURAL_DOMAIN_EXPORT bool RemoveCardinalPoint(Utf8CP name);
    STRUCTURAL_DOMAIN_EXPORT bool SetCardinalPoint(Utf8CP name, DPoint2dCR coordinates);
    STRUCTURAL_DOMAIN_EXPORT bool SetCardinalPoint(Utf8CP name, double x, double y);
    static STRUCTURAL_DOMAIN_EXPORT bool IsStandardCardinalPointName(Utf8CP name);
protected:
    bool FindCardinalPointIndexByName(uint32_t& index, Utf8CP name);
    bool LookupCardinalPointByName(Utf8CP name);
private:
    BE_PROP_NAME(CardinalPoints)
    BE_ECCLASS_NAME(CardinalPointStruct)

    ECN::StandaloneECEnablerPtr GetECEnabler(Utf8CP className);
    uint32_t GetECArrayCount(Utf8CP arrayPropertyName);
    };

//=======================================================================================
//! The ElementHandler for ProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ProfileHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_Profile, Profile, ProfileHandler, Dgn::dgn_ElementHandler::Definition, STRUCTURAL_DOMAIN_EXPORT)
    };

END_BENTLEY_STRUCTURAL_NAMESPACE
