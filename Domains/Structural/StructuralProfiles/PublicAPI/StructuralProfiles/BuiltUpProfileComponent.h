#pragma once
//__PUBLISH_SECTION_START__
#include "StructuralProfilesDefinitions.h"

USING_NAMESPACE_BENTLEY_STRUCTURAL

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE
//TODO: MutiAspect!!! currently this class  not in compile process
struct EXPORT_VTABLE_ATTRIBUTE BuiltUpProfileComponent : Dgn::DgnElement::MultiAspect
{
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_STRUCTURAL_PROFILES_SCHEMA_NAME, STRUCTURAL_PROFILES_CLASS_BuiltUpProfileComponent, Dgn::DgnElement::MultiAspect);

    friend struct BuiltUpProfileComponentHandler;

protected:
    BuiltUpProfileComponent() {};

private:
    Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*) override;
    Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
    Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }

public:
    DECLARE_STRUCTURAL_PROFILES_QUERYCLASS_METHODS(BuiltUpProfileComponent)
    DECLARE_STRUCTURAL_PROFILES_ELEMENT_BASE_GET_METHODS(BuiltUpProfileComponent)

    //STRUCTURAL_DOMAIN_EXPORT static BuiltUpProfileComponentPtr Create(Dgn::PhysicalModelR model);
};

//=======================================================================================
//! The ElementHandler for ConstantProfileHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BuiltUpProfileComponentHandler : Dgn::dgn_AspectHandler::Aspect
{
    DOMAINHANDLER_DECLARE_MEMBERS(STRUCTURAL_PROFILES_CLASS_BuiltUpProfileComponent, BuiltUpProfileComponentHandler, Dgn::dgn_AspectHandler::Aspect, STRUCTURAL_DOMAIN_EXPORT)
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new BuiltUpProfileComponent(); }
};

END_BENTLEY_STRUCTURAL_NAMESPACE